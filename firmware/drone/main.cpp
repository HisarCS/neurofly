/**
 * @file main.cpp
 * @brief NeuroFly Quadcopter Flight Controller Firmware (protocol v2)
 * @target Seeed Studio XIAO ESP32-C6
 *
 * Features:
 * - ESP-NOW receiver with full packet validation (length, magic, version, checksum,
 *   paired-sender MAC, replay protection) and encrypted peer (PMK/LMK from protocol.h)
 * - Built-in 200ms failsafe watchdog
 * - 4-Channel Coreless Motor PWM Driver (LEDC)
 * - MPU6050 IMU: gyro bias calibration, complementary filter, angle->rate cascaded PID
 * - X-configuration motor mixer
 * - VL53L0X / VL53L1X ToF (auto-detected at boot) altitude hold, AI-assist and a simple autonomous takeoff/hover/land mode
 * - Battery monitor (ADC on D2/GPIO2, 2x10k divider)
 * - 10Hz telemetry back to the handheld controller
 *
 * IMU mounting assumed: chip X axis pointing to the drone's front, Y to the left, Z up.
 * If your board is mounted differently, flip the *_SIGN constants below (bench test
 * with props OFF: tilt the nose up and the front motors must NOT speed up more than
 * the rear ones are slowed, i.e. the correction must oppose the tilt).
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <Preferences.h>
#include <VL53L0X.h>
#include <VL53L1X.h>
#include <math.h>
#include "../common/protocol.h"

// ===========================================================================
// Pin definitions (See hardware/schematics/PINOUT.md)
// ===========================================================================
#define PIN_MOTOR_FL  D0 // Front-Left (CW)
#define PIN_MOTOR_FR  D1 // Front-Right (CCW)
#define PIN_MOTOR_RR  D6 // Rear-Right (CW)  (GPIO16)
#define PIN_MOTOR_RL  D3 // Rear-Left (CCW)
#define PIN_SDA       D4
#define PIN_SCL       D5
#define PIN_VBAT      D2 // Battery divider (2x10k): Vbat = 2 * Vadc. GPIO2 = ADC1 (ADC only on GPIO0-6)
#define PIN_BOOT_BTN  9  // XIAO BOOT button (GPIO9): hold 3 s shortly after power-up to clear pairing
#define PIN_LED       D7

// ===========================================================================
// Tunable constants (everything you normally need to tune lives here)
// ===========================================================================

// PWM Configuration
#define PWM_FREQ      20000 // 20kHz for silent coreless DC motor drive
#define PWM_RES       10    // 10-bit resolution (0 - 1023)
#define PWM_MAX       1023

// Loop timing
static const uint32_t LOOP_PERIOD_US = 4000;     // 250 Hz

// IMU
static const uint8_t  MPU_ADDR         = 0x68;
static const float    GYRO_LSB_PER_DPS = 65.5f;  // +-500 dps range
static const float    ACCEL_LSB_PER_G  = 8192.0f;// +-4 g range
static const float    COMP_ALPHA       = 0.98f;  // complementary filter: gyro weight
static const int      GYRO_CAL_SAMPLES = 500;
static const float    GYRO_CAL_MAX_DPS = 8.0f;   // reject calibration if bias bigger (drone moving)
// Axis signs (see mounting note above). Positive roll = left side up, positive pitch = nose
// up, positive yaw rate = clockwise seen from above (matches the mixer comments below).
static const float ROLL_SIGN  =  1.0f;
static const float PITCH_SIGN = -1.0f;
static const float YAW_SIGN   = -1.0f;

// Stick scaling
static const float MAX_ANGLE_DEG   = 25.0f;   // roll/pitch stick +-500 -> +-25 deg
static const float MAX_YAW_RATE    = 120.0f;  // yaw stick +-500 -> +-120 deg/s
static const float AI_MAX_ANGLE_DEG= 10.0f;   // AI_ASSIST roll/pitch correction authority

// Angle (outer) loop: angle error [deg] -> target rate [deg/s]
static const float ANGLE_KP = 4.5f;
static const float MAX_RATE_TARGET = 200.0f;  // deg/s

// Rate (inner) loop PID: rate error [deg/s] -> motor counts (0..1023 scale)
static const float RATE_KP_RP = 0.60f, RATE_KI_RP = 0.50f, RATE_KD_RP = 0.012f; // roll & pitch
static const float RATE_KP_Y  = 1.20f, RATE_KI_Y  = 0.30f, RATE_KD_Y  = 0.0f;   // yaw
static const float I_LIMIT_RP = 80.0f;        // integrator clamp (motor counts)
static const float I_LIMIT_Y  = 60.0f;
static const float CORR_LIMIT_RP = 250.0f;    // max total correction (motor counts)
static const float CORR_LIMIT_Y  = 150.0f;
static const float D_FILTER   = 0.2f;         // D-term low pass (0..1, higher = less filtering)

// Throttle / mixer
static const uint16_t THR_DEADBAND  = 30;     // stick throttle (0-1000) at/below this -> motors zero
static const uint16_t THR_ARM_MAX   = 50;     // must be <= this to arm
static const int      IDLE_SPIN     = 70;     // motor counts when armed above deadband
static const float    TILT_CUTOFF_DEG = 60.0f;// crash protection: disarm beyond this angle

// Battery
static const uint16_t BATT_LOW_MV      = 3300;
static const uint16_t BATT_LOW_CLR_MV  = 3400; // hysteresis
static const uint16_t BATT_CRIT_MV     = 3000;
static const uint16_t BATT_NOSENSE_MV  = 1500; // below this the divider is not connected
static const uint16_t BATT_LOW_LIMIT   = 800;  // max throttle (0-1000) when low
static const uint16_t BATT_CRIT_LIMIT  = 350;  // max throttle (0-1000) when critical (gentle descent)
static const uint32_t BATT_CRIT_DEBOUNCE_MS = 1000;
static const uint32_t BATT_CRIT_CUT_MS = 6000; // after this long critical, motors are cut
#define BATT_REQUIRED 1  // 1: refuse to arm without battery reading (set 0 for USB bench tests)

// Altitude hold (ToF)
static const float    ALT_TARGET_MM = 500.0f;
static const float    ALT_HOVER_THR = 450.0f;  // base throttle (0-1000) near hover; TUNE per battery/weight
static const float    ALT_KP = 0.35f, ALT_KI = 0.15f, ALT_KD = 0.12f; // mm -> throttle units
static const float    ALT_I_LIMIT   = 150.0f;
static const float    ALT_OUT_LIMIT = 250.0f;  // +- around the hover throttle
static const uint16_t ALT_MAX_VALID_MM = 1500; // readings beyond are treated as invalid
static const uint32_t TOF_TIMEOUT_MS   = 300;  // no valid reading for this long -> ToF lost

// Autonomous mission: TAKEOFF to ALT_TARGET_MM -> HOVER AUTO_HOVER_MS -> LAND -> DONE
static const float    AUTO_CLIMB_MM_S  = 250.0f;
static const float    AUTO_DESCEND_MM_S= 150.0f;
static const uint32_t AUTO_HOVER_MS    = 15000;
static const uint32_t AUTO_TAKEOFF_TIMEOUT_MS = 6000;
static const float    AUTO_LAND_DONE_MM= 60.0f;

// Packet / link
static const uint32_t FAILSAFE_MS      = 200;
static const uint32_t REPLAY_RESET_GAP_MS = 300; // after this silence a low packet_id is a controller reboot
static const uint32_t TELEMETRY_PERIOD_MS = 100; // 10 Hz

// Local telemetry flag bits beyond those defined in protocol.h (bits 0..2)
#ifndef TLM_FLAG_ARMED
#define TLM_FLAG_ARMED   3
#define TLM_FLAG_NO_TOF  4
#define TLM_FLAG_NO_IMU  5
#define TLM_FLAG_LOCKOUT 6  // arming locked out (emergency / failsafe / refused arm) until DISARMED seen
#endif
// TLM_FLAG_PAIRING (bit 7) is defined in protocol.h

// ===========================================================================
// State
// ===========================================================================

// --- ESP-NOW receive staging (written in WiFi task, consumed in loop) ---
static portMUX_TYPE rxMux = portMUX_INITIALIZER_UNLOCKED;
static NeuroFlyControlPacket rxStaged;
static volatile bool     rxNew = false;
static volatile uint32_t rxTimeMs = 0;
static volatile int8_t   lastRssi = -100;

// Pairing state. pairedKnown == controller MAC loaded from / stored to NVS. While false the
// drone is in PAIRING STATE: motors stay off and only a DISARMED pairing request is accepted.
static Preferences prefs;
static uint8_t  peerMac[6] = {0};             // the paired controller (valid if pairedKnown)
static volatile bool pairedKnown = false;
static bool     peerAdded = false;            // encrypted peer registered with esp-now
static volatile bool pairReq = false;         // callback saw a valid pairing request
static uint8_t  pairCand[6] = {0};            // MAC of that request
static bool     pairPeerAdded = false;        // temporary UNENCRYPTED peer for the pair-ack
static uint32_t pairStartMs = 0, pairLastAckMs = 0;
static uint32_t bootDoneMs = 0;
static uint32_t bootHeldSince = 0;
static uint32_t lastPacketId = 0;
static bool     havePacketId = false;
static volatile uint32_t lastAcceptedMs = 0;

// --- Flight state ---
static NeuroFlyControlPacket cmd;             // last consumed command
static uint32_t lastRxMs = 0;
static bool     armed = false;
static bool     armLockout = true;            // must see a DISARMED packet before arming
static bool     failsafeTriggered = true;
static uint8_t  requestedMode = MODE_DISARMED;
static uint8_t  effectiveMode = MODE_DISARMED;

// --- IMU ---
static bool  imuPresent = false;
static bool  imuCalibrated = false;
static float gyroBias[3] = {0, 0, 0};
static float rollAngle = 0, pitchAngle = 0;   // deg
static float rollRate = 0, pitchRate = 0, yawRate = 0; // deg/s
static int   imuFailCount = 0;

// --- Battery ---
static uint16_t battMv = 0;
static bool battLow = false, battCritical = false;
static uint32_t battCritSince = 0;

// --- ToF ---
enum TofKind : uint8_t { TOF_NONE = 0, TOF_L0X = 1, TOF_L1X = 2 };
static VL53L0X  tof;                          // used when tofKind == TOF_L0X
static VL53L1X  tof1;                         // used when tofKind == TOF_L1X
static TofKind  tofKind = TOF_NONE;
static bool     tofPresent = false;
static float    altMm = 0;                    // tilt-compensated, filtered
static bool     altValid = false;
static uint32_t lastTofOkMs = 0;
static uint32_t lastTofSampleMs = 0;
static bool     newAltSample = false;

// --- PID state ---
struct RatePid { float i = 0, prevMeas = 0, dFilt = 0; };
static RatePid pidRoll, pidPitch, pidYaw;
static float altI = 0, altPrevMeas = 0, altDFilt = 0, altTarget = 0;

// --- Autonomous state machine ---
enum AutoState : uint8_t { AUTO_IDLE, AUTO_TAKEOFF, AUTO_HOVER, AUTO_LAND, AUTO_DONE };
static AutoState autoState = AUTO_IDLE;
static uint32_t autoStateMs = 0;

// --- Misc ---
static uint32_t lastLoopUs = 0;
static uint32_t lastTlmMs = 0;
static uint32_t lastBattMs = 0;

// ===========================================================================
// Motors
// ===========================================================================
void setMotorOutputs(int fl, int fr, int rr, int rl) {
    ledcWrite(PIN_MOTOR_FL, constrain(fl, 0, PWM_MAX));
    ledcWrite(PIN_MOTOR_FR, constrain(fr, 0, PWM_MAX));
    ledcWrite(PIN_MOTOR_RR, constrain(rr, 0, PWM_MAX));
    ledcWrite(PIN_MOTOR_RL, constrain(rl, 0, PWM_MAX));
}

static void motorsOff() { setMotorOutputs(0, 0, 0, 0); }

// ===========================================================================
// ESP-NOW
// ===========================================================================

// Callback when data is received over ESP-NOW. Runs in the WiFi task: only validates and stages.
void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
    if (len != (int)sizeof(NeuroFlyControlPacket)) return;
    NeuroFlyControlPacket p;
    memcpy(&p, incomingData, sizeof(p));
    if (p.magic != NEUROFLY_MAGIC_BYTE) return;
    if (p.version != NEUROFLY_PROTOCOL_VERSION) return;
    if (p.checksum != neurofly_checksum(&p, sizeof(p))) return;

    if (!pairedKnown) {
        // PAIRING STATE: never fly. Accept only an unencrypted-capable, DISARMED, zero-throttle,
        // non-emergency request; remember the sender so loop() can answer and store it.
        if (p.mode != MODE_DISARMED || p.throttle != 0) return;
        if (p.aux_buttons & ((1 << AUX_BIT_ARM) | (1 << AUX_BIT_EMERGENCY))) return;
        portENTER_CRITICAL(&rxMux);
        if (!pairReq) {
            memcpy(pairCand, recv_info->src_addr, 6);
            pairReq = true;
        }
        portEXIT_CRITICAL(&rxMux);
        return;
    }
    // PAIRED: only the stored controller (encrypted peer, same LMK) is accepted.
    if (memcmp(recv_info->src_addr, peerMac, 6) != 0) return;

    uint32_t nowMs = millis();
    // Replay protection: packet_id must advance. A small id after a long silence is
    // treated as a controller reboot (counter restarted from 0).
    if (havePacketId) {
        int32_t diff = (int32_t)(p.packet_id - lastPacketId); // wraparound safe
        if (diff <= 0) {
            bool reset = (p.packet_id < 100) && (nowMs - lastAcceptedMs > REPLAY_RESET_GAP_MS);
            if (!reset) return;
        }
    }

    portENTER_CRITICAL(&rxMux);
    lastPacketId = p.packet_id;
    havePacketId = true;
    lastAcceptedMs = nowMs;
    rxStaged = p;
    rxTimeMs = nowMs;
    if (recv_info->rx_ctrl) lastRssi = recv_info->rx_ctrl->rssi;
    rxNew = true;
    portEXIT_CRITICAL(&rxMux);
}

static bool addEncryptedPeer(const uint8_t *mac) {
    if (esp_now_is_peer_exist(mac)) esp_now_del_peer(mac);
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, mac, 6);
    peer.channel = 0; // current channel
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = true;
    memcpy(peer.lmk, NEUROFLY_LMK, 16);
    return esp_now_add_peer(&peer) == ESP_OK;
}

static void fillTelemetry(NeuroFlyTelemetryPacket &t, bool pairingAck) {
    t = {};
    t.magic = NEUROFLY_MAGIC_BYTE;
    t.version = NEUROFLY_PROTOCOL_VERSION;
    t.current_mode = armed ? effectiveMode : MODE_DISARMED;
    t.battery_mv = battMv;
    t.pitch_angle = (int16_t)lroundf(pitchAngle * 10.0f);
    t.roll_angle = (int16_t)lroundf(rollAngle * 10.0f);
    t.altitude_mm = (altValid && tofPresent) ? (uint16_t)constrain((int)altMm, 0, 65535) : 0;
    t.rssi = lastRssi;
    uint8_t f = 0;
    if (failsafeTriggered) f |= 1 << TLM_FLAG_FAILSAFE;
    if (battLow || battCritical) f |= 1 << TLM_FLAG_LOW_BATT;
    if (imuCalibrated) f |= 1 << TLM_FLAG_IMU_CALIB;
    if (armed) f |= 1 << TLM_FLAG_ARMED;
    if (!tofPresent || !altValid) f |= 1 << TLM_FLAG_NO_TOF;
    if (!imuPresent) f |= 1 << TLM_FLAG_NO_IMU;
    if (armLockout) f |= 1 << TLM_FLAG_LOCKOUT;
    if (pairingAck) f |= 1 << TLM_FLAG_PAIRING;
    t.flags = f;
    t.checksum = neurofly_checksum(&t, sizeof(t));
}

static void sendTelemetry(uint32_t now) {
    if (!peerAdded || now - lastTlmMs < TELEMETRY_PERIOD_MS) return;
    lastTlmMs = now;
    NeuroFlyTelemetryPacket t;
    fillTelemetry(t, false);
    esp_now_send(peerMac, (const uint8_t *)&t, sizeof(t));
}

// Pairing state machine (runs from loop(), never from the WiFi callback).
//  1. callback saw a valid DISARMED request from pairCand
//  2. register pairCand as an UNENCRYPTED peer and send pair-ack telemetry (TLM_FLAG_PAIRING)
//  3. after a short settle time store the MAC in NVS and swap to an ENCRYPTED peer
static void pairingService(uint32_t now) {
    if (pairedKnown || !pairReq) return;
    if (!pairPeerAdded) {
        esp_now_peer_info_t peer = {};
        memcpy(peer.peer_addr, pairCand, 6);
        peer.channel = 0;
        peer.ifidx = WIFI_IF_STA;
        peer.encrypt = false;
        if (esp_now_is_peer_exist(pairCand)) esp_now_del_peer(pairCand);
        if (esp_now_add_peer(&peer) != ESP_OK) { pairReq = false; Serial.println("Pairing: add peer failed"); return; }
        pairPeerAdded = true;
        pairStartMs = now;
        pairLastAckMs = now - 100;
    }
    if (now - pairLastAckMs >= 20) {
        pairLastAckMs = now;
        NeuroFlyTelemetryPacket t;
        fillTelemetry(t, true);
        esp_now_send(pairCand, (const uint8_t *)&t, sizeof(t));
    }
    if (now - pairStartMs >= 200) {
        esp_now_del_peer(pairCand);
        pairPeerAdded = false;
        memcpy(peerMac, pairCand, 6);
        prefs.putBytes("ctrl", peerMac, 6);
        peerAdded = addEncryptedPeer(peerMac);
        havePacketId = false;
        pairReq = false;
        pairedKnown = true;
        Serial.printf("Paired with controller %02X:%02X:%02X:%02X:%02X:%02X (encrypted peer %s)\n",
                      peerMac[0], peerMac[1], peerMac[2], peerMac[3], peerMac[4], peerMac[5],
                      peerAdded ? "ok" : "FAILED");
    }
}

// Hold the BOOT button (GPIO9) for 3 s during the first 5 s after start-up to clear pairing.
// (Do NOT hold it while powering on: that enters the ROM bootloader.)
static void checkPairingClear(uint32_t now) {
    if (now - bootDoneMs > 5000 || armed) return;
    if (digitalRead(PIN_BOOT_BTN) == LOW) {
        if (bootHeldSince == 0) bootHeldSince = now ? now : 1;
        if (now - bootHeldSince >= 3000) {
            motorsOff();
            prefs.remove("ctrl");
            Serial.println("Pairing cleared. Restarting into pairing state.");
            delay(100);
            ESP.restart();
        }
    } else {
        bootHeldSince = 0;
    }
}

// Fatal init error: motors stay off, LED blinks "three quick flashes, pause" forever.
static void fatalErrorBlink() {
    motorsOff();
    while (true) {
        for (int i = 0; i < 3; i++) {
            digitalWrite(PIN_LED, HIGH); delay(100);
            digitalWrite(PIN_LED, LOW);  delay(150);
        }
        delay(800);
        motorsOff();
    }
}

// ===========================================================================
// MPU6050
// ===========================================================================
static bool mpuWrite(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

// Reads accel [g] and gyro [deg/s] (raw, unbiased). Returns false on I2C failure.
static bool mpuRead(float acc[3], float gyro[3]) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom((int)MPU_ADDR, 14) != 14) return false;
    int16_t v[7];
    for (int i = 0; i < 7; i++) {
        uint8_t hi = Wire.read();
        uint8_t lo = Wire.read();
        v[i] = (int16_t)((hi << 8) | lo);
    }
    acc[0] = v[0] / ACCEL_LSB_PER_G;
    acc[1] = v[1] / ACCEL_LSB_PER_G;
    acc[2] = v[2] / ACCEL_LSB_PER_G;
    // v[3] is temperature
    gyro[0] = v[4] / GYRO_LSB_PER_DPS;
    gyro[1] = v[5] / GYRO_LSB_PER_DPS;
    gyro[2] = v[6] / GYRO_LSB_PER_DPS;
    return true;
}

static void accelAngles(const float a[3], float &roll, float &pitch) {
    roll  = ROLL_SIGN  * atan2f(a[1], a[2]) * RAD_TO_DEG;
    pitch = PITCH_SIGN * atan2f(-a[0], sqrtf(a[1] * a[1] + a[2] * a[2])) * RAD_TO_DEG;
}

static bool imuInit() {
    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(400000);
    Wire.setTimeOut(5);

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x75); // WHO_AM_I
    if (Wire.endTransmission(false) != 0 || Wire.requestFrom((int)MPU_ADDR, 1) != 1) return false;
    uint8_t who = Wire.read();
    if (who != 0x68 && who != 0x70 && who != 0x72 && who != 0x98) return false; // genuine / clones

    if (!mpuWrite(0x6B, 0x01)) return false; // wake, PLL with X gyro
    delay(100);
    mpuWrite(0x1A, 0x03); // DLPF ~44Hz
    mpuWrite(0x1B, 0x08); // gyro +-500 dps
    mpuWrite(0x1C, 0x08); // accel +-4 g
    mpuWrite(0x19, 0x00); // sample rate divider (1kHz internal / 1)
    delay(50);
    return true;
}

// Average gyro at rest to find bias; fails if the drone is moving. Returns true on success.
static bool imuCalibrate() {
    for (int attempt = 0; attempt < 3; attempt++) {
        double sum[3] = {0, 0, 0};
        float minV[3] = {1e9f, 1e9f, 1e9f}, maxV[3] = {-1e9f, -1e9f, -1e9f};
        float acc[3], gyro[3], accSum[3] = {0, 0, 0};
        int n = 0;
        for (int i = 0; i < GYRO_CAL_SAMPLES; i++) {
            if (mpuRead(acc, gyro)) {
                for (int k = 0; k < 3; k++) {
                    sum[k] += gyro[k];
                    accSum[k] += acc[k];
                    minV[k] = min(minV[k], gyro[k]);
                    maxV[k] = max(maxV[k], gyro[k]);
                }
                n++;
            }
            delay(3);
        }
        if (n < GYRO_CAL_SAMPLES / 2) continue;
        bool still = true;
        for (int k = 0; k < 3; k++) {
            if ((maxV[k] - minV[k]) > GYRO_CAL_MAX_DPS * 2 || fabs(sum[k] / n) > GYRO_CAL_MAX_DPS) still = false;
        }
        if (!still) { Serial.println("IMU calibration: drone moving, retrying..."); continue; }
        for (int k = 0; k < 3; k++) { gyroBias[k] = sum[k] / n; accSum[k] /= n; }
        accelAngles(accSum, rollAngle, pitchAngle); // start filter from the accel attitude
        return true;
    }
    return false;
}

// One IMU step: read, bias-correct, complementary filter.
static bool imuUpdate(float dt) {
    float acc[3], gyro[3];
    if (!mpuRead(acc, gyro)) return false;
    rollRate  = ROLL_SIGN  * (gyro[0] - gyroBias[0]);
    pitchRate = PITCH_SIGN * (gyro[1] - gyroBias[1]);
    yawRate   = YAW_SIGN   * (gyro[2] - gyroBias[2]);

    float accRoll, accPitch;
    accelAngles(acc, accRoll, accPitch);
    rollAngle  = COMP_ALPHA * (rollAngle  + rollRate  * dt) + (1.0f - COMP_ALPHA) * accRoll;
    pitchAngle = COMP_ALPHA * (pitchAngle + pitchRate * dt) + (1.0f - COMP_ALPHA) * accPitch;
    return true;
}

// ===========================================================================
// Battery
// ===========================================================================
static void batteryUpdate(uint32_t now) {
    if (now - lastBattMs < 20) return;
    lastBattMs = now;
    uint16_t mv = (uint16_t)(analogReadMilliVolts(PIN_VBAT) * 2);
    battMv = (battMv == 0) ? mv : (uint16_t)(battMv * 0.9f + mv * 0.1f); // low-pass vs motor sag noise

    bool noSense = battMv < BATT_NOSENSE_MV;
    if (noSense) { // divider not connected: treat as critical when BATT_REQUIRED
        battLow = BATT_REQUIRED;
        battCritical = BATT_REQUIRED;
        battCritSince = battCritSince ? battCritSince : now;
        return;
    }
    if (battMv < BATT_LOW_MV) battLow = true;
    else if (battMv > BATT_LOW_CLR_MV) battLow = false;

    if (battMv < BATT_CRIT_MV) {
        if (battCritSince == 0) battCritSince = now;
        if (now - battCritSince > BATT_CRIT_DEBOUNCE_MS) battCritical = true;
    } else {
        battCritSince = 0;
        if (battMv > BATT_LOW_CLR_MV) battCritical = false;
    }
}

// ===========================================================================
// ToF (VL53L0X or VL53L1X, auto-detected on I2C 0x29; both expose the same interface below)
// ===========================================================================
static const uint8_t TOF_ADDR = 0x29;

// Identify the sensor from its model-id register. Returns TOF_NONE if nothing answers/matches.
static TofKind tofDetect() {
    // VL53L0X: 8-bit register 0xC0 == 0xEE
    Wire.beginTransmission(TOF_ADDR);
    Wire.write(0xC0);
    if (Wire.endTransmission(false) == 0 && Wire.requestFrom((int)TOF_ADDR, 1) == 1) {
        if (Wire.read() == 0xEE) return TOF_L0X;
    }
    // VL53L1X: 16-bit register 0x010F == 0xEACC
    Wire.beginTransmission(TOF_ADDR);
    Wire.write(0x01);
    Wire.write(0x0F);
    if (Wire.endTransmission(false) == 0 && Wire.requestFrom((int)TOF_ADDR, 2) == 2) {
        uint8_t hi = Wire.read();
        uint8_t lo = Wire.read();
        if ((uint16_t)((hi << 8) | lo) == 0xEACC) return TOF_L1X;
    }
    return TOF_NONE;
}

static bool tofInit() {
    tofKind = tofDetect();
    if (tofKind == TOF_L0X) {
        tof.setTimeout(50);
        if (!tof.init()) { tofKind = TOF_NONE; return false; }
        tof.setMeasurementTimingBudget(20000); // 20ms -> ~50Hz
        tof.startContinuous();
        return true;
    }
    if (tofKind == TOF_L1X) {
        tof1.setTimeout(50);
        if (!tof1.init()) { tofKind = TOF_NONE; return false; }
        tof1.setDistanceMode(VL53L1X::Short);          // best ambient immunity for <1.5 m hover
        tof1.setMeasurementTimingBudget(20000);        // 20 ms
        tof1.startContinuous(20);                      // back-to-back, 20 ms period
        return true;
    }
    return false;
}

// Non-blocking: returns true and fills mm only if a NEW valid measurement was available.
static bool tofReadRaw(uint16_t &mm) {
    if (tofKind == TOF_L0X) {
        if ((tof.readReg(VL53L0X::RESULT_INTERRUPT_STATUS) & 0x07) == 0) return false;
        mm = tof.readRangeContinuousMillimeters();
        return !tof.timeoutOccurred();
    }
    if (tofKind == TOF_L1X) {
        if (!tof1.dataReady()) return false;
        mm = tof1.read(false);                         // data is ready, so this does not block
        return !tof1.timeoutOccurred() && tof1.ranging_data.range_status == VL53L1X::RangeValid;
    }
    return false;
}

static void tofUpdate(uint32_t now) {
    if (!tofPresent) return;
    if (now - lastTofSampleMs >= 10) {
        lastTofSampleMs = now;
        uint16_t mm;
        if (tofReadRaw(mm) && mm <= ALT_MAX_VALID_MM) {
            float tilt = cosf(rollAngle * DEG_TO_RAD) * cosf(pitchAngle * DEG_TO_RAD);
            float h = mm * max(tilt, 0.5f);
            altMm = altValid ? (altMm * 0.5f + h * 0.5f) : h;
            altValid = true;
            lastTofOkMs = now;
            newAltSample = true;
        }
    }
    if (altValid && now - lastTofOkMs > TOF_TIMEOUT_MS) altValid = false;
}

// ===========================================================================
// Control helpers
// ===========================================================================
static float ratePid(RatePid &s, float target, float meas, float dt,
                     float kp, float ki, float kd, float iLim, float outLim, bool integrate) {
    float err = target - meas;
    if (integrate) s.i = constrain(s.i + err * ki * dt, -iLim, iLim);
    // Derivative on measurement (no kick on setpoint changes), low-pass filtered
    float d = -(meas - s.prevMeas) / dt;
    s.dFilt += D_FILTER * (d - s.dFilt);
    s.prevMeas = meas;
    return constrain(kp * err + s.i + kd * s.dFilt, -outLim, outLim);
}

static void resetPids() {
    pidRoll = RatePid(); pidPitch = RatePid(); pidYaw = RatePid();
    altI = 0; altDFilt = 0; altPrevMeas = altMm;
}

static void disarm() {
    armed = false;
    requestedMode = MODE_DISARMED;
    autoState = AUTO_IDLE;
    resetPids();
    motorsOff();
}

static bool canArm() {
    if (!imuPresent || !imuCalibrated) return false;
    if (failsafeTriggered) return false;
    if (battCritical) return false;
    return true;
}

// Handle a freshly received (validated) control packet.
static void processCommand(const NeuroFlyControlPacket &p) {
    bool emergency = (p.aux_buttons >> AUX_BIT_EMERGENCY) & 1;
    if (emergency) {
        // Immediate disarm; stays latched until a DISARMED packet without emergency is seen.
        disarm();
        armLockout = true;
        return;
    }
    if (p.mode == MODE_DISARMED || p.mode > MODE_AI_ASSIST) {
        if (armed) disarm();
        armLockout = false; // DISARMED seen: arming allowed again
        return;
    }
    if (armed) {
        requestedMode = p.mode; // switching among armed modes is allowed in flight
        return;
    }
    // DISARMED -> armed transition requested
    if (armLockout) return;
    if (p.throttle <= THR_ARM_MAX && canArm()) {
        armed = true;
        requestedMode = p.mode;
        resetPids();
        Serial.println("ARMED");
    } else {
        armLockout = true; // refused (throttle high / IMU / battery): need a DISARMED packet first
        Serial.println("Arm refused");
    }
}

// Autonomous mission state machine. Returns altitude target, sets throttleOn=false when finished.
static float autonomousStep(uint32_t now, float dt, bool &active) {
    active = true;
    switch (autoState) {
        case AUTO_IDLE:
            autoState = AUTO_TAKEOFF; autoStateMs = now;
            altTarget = 0; altI = 0;
            // fallthrough
        case AUTO_TAKEOFF:
            altTarget = min(altTarget + AUTO_CLIMB_MM_S * dt, ALT_TARGET_MM);
            if ((altTarget >= ALT_TARGET_MM && fabsf(altMm - ALT_TARGET_MM) < 60.0f)) {
                autoState = AUTO_HOVER; autoStateMs = now;
            } else if (now - autoStateMs > AUTO_TAKEOFF_TIMEOUT_MS) {
                autoState = AUTO_LAND; autoStateMs = now; // could not reach altitude: abort
            }
            break;
        case AUTO_HOVER:
            altTarget = ALT_TARGET_MM;
            if (now - autoStateMs > AUTO_HOVER_MS) { autoState = AUTO_LAND; autoStateMs = now; }
            break;
        case AUTO_LAND:
            altTarget = max(altTarget - AUTO_DESCEND_MM_S * dt, 0.0f);
            if (altMm < AUTO_LAND_DONE_MM) { autoState = AUTO_DONE; autoStateMs = now; }
            break;
        case AUTO_DONE:
            active = false; // motors off until mode changes or disarm
            break;
    }
    return altTarget;
}

// Altitude PID -> throttle (0..1000)
static float altitudeThrottle(float target, float dt) {
    if (newAltSample) {
        newAltSample = false;
        float d = -(altMm - altPrevMeas) / dt;
        altDFilt += 0.3f * (d - altDFilt);
        altPrevMeas = altMm;
    }
    float err = target - altMm;
    altI = constrain(altI + err * ALT_KI * dt, -ALT_I_LIMIT, ALT_I_LIMIT);
    float out = constrain(ALT_KP * err + altI + ALT_KD * altDFilt, -ALT_OUT_LIMIT, ALT_OUT_LIMIT);
    return ALT_HOVER_THR + out;
}

static uint16_t batteryThrottleLimit() {
    if (battCritical) return BATT_CRIT_LIMIT;
    if (battLow) return BATT_LOW_LIMIT;
    return 1000;
}

// ===========================================================================
// Arduino entry points
// ===========================================================================
void setup() {
    Serial.begin(115200);

    // Motors first, so they are guaranteed off.
    ledcAttach(PIN_MOTOR_FL, PWM_FREQ, PWM_RES);
    ledcAttach(PIN_MOTOR_FR, PWM_FREQ, PWM_RES);
    ledcAttach(PIN_MOTOR_RR, PWM_FREQ, PWM_RES);
    ledcAttach(PIN_MOTOR_RL, PWM_FREQ, PWM_RES);
    motorsOff();

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
    pinMode(PIN_VBAT, INPUT);

    delay(300);
    Serial.printf("NeuroFly drone firmware %s (protocol v%d)\n",
                  NEUROFLY_FIRMWARE_VERSION, NEUROFLY_PROTOCOL_VERSION);

    // IMU (required for arming)
    imuPresent = imuInit();
    if (imuPresent) {
        Serial.println("MPU6050 found, calibrating gyro (keep drone still)...");
        imuCalibrated = imuCalibrate();
        Serial.println(imuCalibrated ? "IMU calibrated." : "IMU calibration FAILED: arming disabled.");
    } else {
        Serial.println("MPU6050 NOT found: arming disabled.");
    }

    // ToF (optional: needed for ALT_HOLD / AUTONOMOUS / AI_ASSIST)
    tofPresent = tofInit();
    if (tofPresent) Serial.println(tofKind == TOF_L1X ? "VL53L1X found." : "VL53L0X found.");
    else Serial.println("No VL53L0X/VL53L1X found: altitude modes fall back to manual.");

    // Wi-Fi Station Mode for ESP-NOW
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW Init Failed!");
        fatalErrorBlink(); // never returns, motors stay off
    }
    esp_now_set_pmk((const uint8_t *)NEUROFLY_PMK);
    // Pairing: stored controller MAC => encrypted peer immediately; otherwise pairing state.
    prefs.begin("neurofly", false);
    pairedKnown = (prefs.getBytes("ctrl", peerMac, 6) == 6);
    if (pairedKnown) {
        peerAdded = addEncryptedPeer(peerMac);
        Serial.printf("Paired controller %02X:%02X:%02X:%02X:%02X:%02X, encrypted peer %s\n",
                      peerMac[0], peerMac[1], peerMac[2], peerMac[3], peerMac[4], peerMac[5],
                      peerAdded ? "ok" : "FAILED");
    } else {
        Serial.println("PAIRING STATE: motors disabled until a controller pairs.");
    }
    esp_now_register_recv_cb(onDataRecv);
    pinMode(PIN_BOOT_BTN, INPUT_PULLUP);

    batteryUpdate(millis());
    bootDoneMs = millis();
    lastLoopUs = micros();
    Serial.println("NeuroFly Flight Controller Initialized.");
}

void loop() {
    uint32_t nowUs = micros();
    if (nowUs - lastLoopUs < LOOP_PERIOD_US) return; // fixed ~250Hz scheduling, no delay()
    float dt = (nowUs - lastLoopUs) * 1e-6f;
    lastLoopUs = nowUs;
    dt = constrain(dt, 0.001f, 0.02f);
    uint32_t now = millis();

    // --- Sensors ---
    if (imuPresent) {
        if (imuUpdate(dt)) imuFailCount = 0;
        else if (++imuFailCount > 10 && armed) { // lost IMU in flight
            disarm(); armLockout = true;
            Serial.println("IMU read failure: disarmed");
        }
    }
    batteryUpdate(now);
    tofUpdate(now);
    pairingService(now);
    checkPairingClear(now);

    // --- Commands ---
    if (rxNew && pairedKnown) {
        NeuroFlyControlPacket p;
        portENTER_CRITICAL(&rxMux);
        p = rxStaged;
        lastRxMs = rxTimeMs;
        rxNew = false;
        portEXIT_CRITICAL(&rxMux);
        cmd = p;
        failsafeTriggered = false;
        processCommand(p);
    }

    // Safety Failsafe: cut power if no packets received for > 200ms
    // signed diff: a packet may be stamped (callback) after `now` was sampled this iteration
    if (!failsafeTriggered && (int32_t)(now - lastRxMs) > (int32_t)FAILSAFE_MS) {
        failsafeTriggered = true;
        if (armed) Serial.println("FAILSAFE: link lost");
        disarm();
        armLockout = true;
    }

    // Crash protection and prolonged critical battery
    if (armed && (fabsf(rollAngle) > TILT_CUTOFF_DEG || fabsf(pitchAngle) > TILT_CUTOFF_DEG)) {
        disarm(); armLockout = true;
        Serial.println("Tilt limit exceeded: disarmed");
    }
    if (armed && battCritical && now - battCritSince > BATT_CRIT_CUT_MS) {
        disarm(); armLockout = true;
        Serial.println("Battery critical: landed/disarmed");
    }

    // --- Status LED: solid = armed, fast blink = sensor fault, slow blink = ready ---
    if (armed) digitalWrite(PIN_LED, HIGH);
    else if (!pairedKnown) { uint32_t ph = now % 1000; digitalWrite(PIN_LED, (ph < 80 || (ph >= 200 && ph < 280)) ? HIGH : LOW); } // double flash = pairing state
    else if (!imuPresent || !imuCalibrated) digitalWrite(PIN_LED, (now / 100) & 1);
    else digitalWrite(PIN_LED, ((now % 1000) < 100) ? HIGH : LOW);

    sendTelemetry(now);

    if (!armed) {
        effectiveMode = MODE_DISARMED;
        motorsOff();
        return;
    }

    // --- Mode selection (alt modes need a working ToF) ---
    uint8_t mode = requestedMode;
    bool tofOk = tofPresent && altValid;
    if (mode >= MODE_ALT_HOLD && !tofOk) mode = MODE_ARMED_MANUAL; // fall back to manual (NO_TOF flag set)
    if (mode != effectiveMode) {
        // Re-initialise controllers on every mode change for bumpless transfer
        altI = 0; altDFilt = 0; altPrevMeas = altMm;
        altTarget = (mode == MODE_AUTONOMOUS) ? 0 : ALT_TARGET_MM;
        autoState = AUTO_IDLE;
        effectiveMode = mode;
    }

    // --- Throttle & attitude targets ---
    float thr = cmd.throttle;                        // 0..1000
    float tRoll = 0, tPitch = 0, tYawRate = 0;       // deg, deg, deg/s
    bool motorsActive = true;

    float rollStick  = constrain((int)cmd.roll,  -500, 500) / 500.0f;
    float pitchStick = constrain((int)cmd.pitch, -500, 500) / 500.0f;
    float yawStick   = constrain((int)cmd.yaw,   -500, 500) / 500.0f;

    switch (mode) {
        case MODE_ARMED_MANUAL:
            tRoll = rollStick * MAX_ANGLE_DEG;
            tPitch = pitchStick * MAX_ANGLE_DEG;
            tYawRate = yawStick * MAX_YAW_RATE;
            motorsActive = thr > THR_DEADBAND;
            break;
        case MODE_ALT_HOLD:
            tRoll = rollStick * MAX_ANGLE_DEG;
            tPitch = pitchStick * MAX_ANGLE_DEG;
            tYawRate = yawStick * MAX_YAW_RATE;
            motorsActive = cmd.throttle > THR_DEADBAND;   // stick down = cut (landing)
            if (motorsActive) thr = altitudeThrottle(ALT_TARGET_MM, dt);
            break;
        case MODE_AI_ASSIST:
            // Roll/pitch/yaw from the packet are small vision-script corrections on top of alt hold.
            tRoll = rollStick * AI_MAX_ANGLE_DEG;
            tPitch = pitchStick * AI_MAX_ANGLE_DEG;
            tYawRate = yawStick * MAX_YAW_RATE;
            motorsActive = cmd.throttle > THR_DEADBAND;
            if (motorsActive) thr = altitudeThrottle(ALT_TARGET_MM, dt);
            break;
        case MODE_AUTONOMOUS: {
            float target = autonomousStep(now, dt, motorsActive);
            if (motorsActive) thr = altitudeThrottle(target, dt);
            break;
        }
    }

    thr = min(thr, (float)batteryThrottleLimit());

    if (!motorsActive) {
        resetPids();
        motorsOff();
        return;
    }

    // --- Cascaded stabiliser: angle P -> rate PID ---
    float rollRateTgt  = constrain(ANGLE_KP * (tRoll  - rollAngle),  -MAX_RATE_TARGET, MAX_RATE_TARGET);
    float pitchRateTgt = constrain(ANGLE_KP * (tPitch - pitchAngle), -MAX_RATE_TARGET, MAX_RATE_TARGET);
    float rollOut  = ratePid(pidRoll,  rollRateTgt,  rollRate,  dt, RATE_KP_RP, RATE_KI_RP, RATE_KD_RP,
                             I_LIMIT_RP, CORR_LIMIT_RP, thr > THR_DEADBAND);
    float pitchOut = ratePid(pidPitch, pitchRateTgt, pitchRate, dt, RATE_KP_RP, RATE_KI_RP, RATE_KD_RP,
                             I_LIMIT_RP, CORR_LIMIT_RP, thr > THR_DEADBAND);
    float yawOut   = ratePid(pidYaw,   tYawRate,     yawRate,   dt, RATE_KP_Y, RATE_KI_Y, RATE_KD_Y,
                             I_LIMIT_Y, CORR_LIMIT_Y, thr > THR_DEADBAND);

    // --- Quadcopter Motor Mixer (X-Configuration) ---
    // Front-Left  (CW)  = Throttle + Pitch + Roll - Yaw
    // Front-Right (CCW) = Throttle + Pitch - Roll + Yaw
    // Rear-Right  (CW)  = Throttle - Pitch - Roll - Yaw
    // Rear-Left   (CCW) = Throttle - Pitch + Roll + Yaw
    // Idle spin while armed above the deadband; throttle 0..1000 maps to IDLE_SPIN..1023.
    float base = IDLE_SPIN + constrain(thr, 0.0f, 1000.0f) * (PWM_MAX - IDLE_SPIN) / 1000.0f;
    int mFL = (int)(base + pitchOut + rollOut - yawOut);
    int mFR = (int)(base + pitchOut - rollOut + yawOut);
    int mRR = (int)(base - pitchOut - rollOut - yawOut);
    int mRL = (int)(base - pitchOut + rollOut + yawOut);

    // Never let a correction drive an armed motor fully off (keeps authority), clamp at max
    mFL = constrain(mFL, IDLE_SPIN / 2, PWM_MAX);
    mFR = constrain(mFR, IDLE_SPIN / 2, PWM_MAX);
    mRR = constrain(mRR, IDLE_SPIN / 2, PWM_MAX);
    mRL = constrain(mRL, IDLE_SPIN / 2, PWM_MAX);

    setMotorOutputs(mFL, mFR, mRR, mRL);
}
