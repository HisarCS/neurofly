/**
 * @file main.cpp
 * @brief NeuroFly Smart Remote Controller Firmware (protocol v2)
 * @target Seeed Studio XIAO ESP32-C6, Arduino-ESP32 core 3.x
 *
 * Features:
 * - Analog gimbals OR push buttons (compile-time INPUT_MODE)
 * - Encrypted ESP-NOW unicast link to ONE paired drone (MAC stored in NVS)
 * - Pairing mode (hold MODE button while powering on): unencrypted DISARMED broadcast;
 *   the first valid telemetry reply flagged TLM_FLAG_PAIRING is stored as the paired drone
 * - Streams JSON state over USB serial for the NeuroFly Web Simulator
 * - Optional inbound serial line {"vr":<roll>,"vp":<pitch>} (ArUco vision
 *   script) applied as corrections in AI_ASSIST mode
 * - 0.96" SSD1306 status display (optional; firmware runs without it)
 *
 * ---------------------------------------------------------------------------
 * PIN MAP (authoritative)
 *   D0 throttle   D1 yaw   D2 pitch   D3 roll        (BUTTONS mode only; GPIO0,1,2,21)
 *   ADS1115 A0 thr, A1 yaw, A2 pitch, A3 roll        (ANALOG_GIMBALS mode, I2C 0x48)
 *   D8 ARM button (GPIO19) D9 MODE button (GPIO20)           (active LOW, pull-up)
 *   D4 SDA / D5 SCL        OLED (I2C, 0x3C, 400 kHz)
 *
 * INPUT_MODE selection (build_flags in platformio.ini, e.g. -DINPUT_MODE=1)
 *   INPUT_BUTTONS        (0, default)  D0..D3 are active-LOW push buttons.
 *   INPUT_ANALOG_GIMBALS (1)           4 gimbal wipers (0..3.3 V) on an external
 *                                      ADS1115 (A0..A3); D0..D3 are unused. Keep the throttle stick at
 *                                      the bottom and the others centred at
 *                                      power-on: that is the calibration.
 *
 * BUTTON MODE mapping (only 4 stick buttons exist, so MODE acts as a shift key)
 *   +-----------+-----------------------+-------------------------------+
 *   | Pin       | normal                | while MODE button is held     |
 *   +-----------+-----------------------+-------------------------------+
 *   | D0        | throttle UP (ramp)    | yaw   +250 (right)            |
 *   | D1        | throttle DOWN (ramp)  | yaw   -250 (left)             |
 *   | D2        | pitch +250            | roll  +250 (right)            |
 *   | D3        | pitch -250            | roll  -250 (left)             |
 *   +-----------+-----------------------+-------------------------------+
 *   (Mode change happens on a SHORT press-release of MODE that did not use
 *    the shift function.)
 *
 * ARM button: short press toggles arm/disarm. Arming is refused unless the
 * throttle is at minimum. Holding ARM >= 1 s is EMERGENCY CUT: disarm, throttle
 * 0, AUX_BIT_EMERGENCY set in packets for the next 500 ms.
 *
 * MODE cycling: Manual -> AltHold -> Autonomous -> AI Assist -> Manual.
 *   Disarmed: full cycle. Armed (safety): only Manual <-> AltHold; if armed in
 *   Autonomous/AI the press drops straight back to Manual (pilot takes over).
 *   Arming from Autonomous/AI selects Manual first.
 * ---------------------------------------------------------------------------
 */

#define INPUT_BUTTONS         0
#define INPUT_ANALOG_GIMBALS  1
#ifndef INPUT_MODE
#define INPUT_MODE INPUT_BUTTONS
#endif

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#if INPUT_MODE == 1
#include <Adafruit_ADS1X15.h>
#endif
#include "../common/protocol.h"

// ---- Pins ------------------------------------------------------------------
// Buttons mode: D0..D3 (GPIO0,1,2,21) are plain digital inputs.
// Analog mode: the ESP32-C6 ADC only exists on GPIO0-6 (XIAO D0-D2), so 4 axes are read through
// an external ADS1115 (I2C 0x48, same bus as the OLED): A0 throttle, A1 yaw, A2 pitch, A3 roll.
#define PIN_THROTTLE  D0
#define PIN_YAW       D1
#define PIN_PITCH     D2
#define PIN_ROLL      D3
#define PIN_ARM       D8   // GPIO19
#define PIN_MODE      D9   // GPIO20
#define ADS_ADDR      0x48
#define ADS_CH_THROTTLE 0
#define ADS_CH_YAW      1
#define ADS_CH_PITCH    2
#define ADS_CH_ROLL     3

// ---- Tunables --------------------------------------------------------------
#define TX_PERIOD_MS          25      // 40 Hz
#define OLED_PERIOD_MS        200     // 5 Hz
#define LINK_TIMEOUT_MS       500
#define DEBOUNCE_MS           25
#define EMERGENCY_HOLD_MS     1000
#define EMERGENCY_LATCH_MS    500
#define MODE_SHORT_PRESS_MS   800
#define ARM_MAX_THROTTLE      30      // arm only if throttle <= 3 %
#define BTN_THROTTLE_STEP     5       // per TX tick while held
#define BTN_DEFLECT           250
#define VISION_TIMEOUT_MS     300
#define VISION_CLAMP          300
#define ADC_DEADZONE          120     // counts (of 4095)
#define ADC_EDGE_MARGIN       80
// Flip to -1 if a gimbal axis is mounted reversed
#define INVERT_ROLL   1
#define INVERT_PITCH  1
#define INVERT_YAW    1

static const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ---- State -----------------------------------------------------------------
NeuroFlyControlPacket txPacket;
NeuroFlyTelemetryPacket rxTelemetry;       // guarded by rxMux
portMUX_TYPE rxMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t lastRxMs = 0;            // 0 = never
volatile int8_t lastRxRssi = 0;
volatile bool pairFound = false;
uint8_t pairMac[6];

uint8_t droneMac[6];
bool hasPairedMac = false;
bool pairingMode = false;
bool espOk = false;
uint32_t lastEspRetry = 0;
Preferences prefs;

bool isArmed = false;
uint8_t selectedMode = MODE_ARMED_MANUAL;  // used while armed; disarmed packets say DISARMED
uint16_t rawThrottle = 0;                  // stick/button throttle 0..1000
int16_t stickRoll = 0, stickPitch = 0, stickYaw = 0;
uint32_t packetCounter = 0;
uint32_t emergencyStart = 0;              // 0 = none; millis() at emergency press (never 0)

// analog calibration
int adcThrMin = 150, adcCenterRoll = 2048, adcCenterPitch = 2048, adcCenterYaw = 2048;

// vision corrections
int16_t visionRoll = 0, visionPitch = 0;
uint32_t visionStamp = 0;
char serialBuf[64];
uint8_t serialLen = 0;
bool serialOverflow = false;               // current line too long: discard it up to the newline

Adafruit_SSD1306 display(128, 64, &Wire, -1);
bool oledOk = false;

// ---- Non-blocking debounced button -----------------------------------------
struct Button {
    uint8_t pin;
    bool pressed = false;       // debounced
    bool rawLast = false;
    uint32_t rawChangedAt = 0;
    uint32_t pressedAt = 0;
    bool fell = false, rose = false;   // one-shot edges, valid after update()

    void begin(uint8_t p) {
        pin = p;
        pinMode(pin, INPUT_PULLUP);
        rawLast = (digitalRead(pin) == LOW);
        pressed = rawLast;
        rawChangedAt = millis();
        pressedAt = pressed ? millis() : 0;
    }
    void update(uint32_t now) {
        fell = rose = false;
        bool raw = (digitalRead(pin) == LOW);
        if (raw != rawLast) { rawLast = raw; rawChangedAt = now; }
        if (raw != pressed && (now - rawChangedAt) >= DEBOUNCE_MS) {
            pressed = raw;
            if (pressed) { fell = true; pressedAt = now; } else { rose = true; }
        }
    }
};

Button btnArm, btnMode;
#if INPUT_MODE == INPUT_BUTTONS
Button btnD0, btnD1, btnD2, btnD3;
#endif
bool modeShiftUsed = false;
bool modeIgnoreRelease = false;
bool armLongHandled = false;

// ---- Helpers ----------------------------------------------------------------
static const char *modeName(uint8_t m) {
    switch (m) {
        case MODE_DISARMED:    return "DISARMED";
        case MODE_ARMED_MANUAL:return "MANUAL";
        case MODE_ALT_HOLD:    return "ALT HOLD";
        case MODE_AUTONOMOUS:  return "AUTONOMOUS";
        case MODE_AI_ASSIST:   return "AI ASSIST";
    }
    return "?";
}

static int16_t clamp16(int v, int lo, int hi) { return (int16_t)(v < lo ? lo : (v > hi ? hi : v)); }

static bool linkOk(uint32_t now) {
    uint32_t t = lastRxMs;
    return t != 0 && (int32_t)(now - t) <= (int32_t)LINK_TIMEOUT_MS; // signed: callback may stamp after `now`
}

#if INPUT_MODE == INPUT_ANALOG_GIMBALS
static Adafruit_ADS1115 ads;
static bool adsOk = false;

// One single-ended ADS1115 reading scaled to the 0..4095 "ADC count" range used below
// (gain 1: +-4.096 V, 0.125 mV/LSB; a 3.3 V wiper = 26400 raw).
static int readAxis(uint8_t ch) {
    if (!adsOk) return 0;
    int16_t raw = ads.readADC_SingleEnded(ch);
    if (raw < 0) raw = 0;
    long c = (long)raw * 4095 / 26400;
    return (int)(c > 4095 ? 4095 : c);
}

static int readAdcAvg(uint8_t ch, int n = 16) {
    long s = 0;
    for (int i = 0; i < n; i++) { s += readAxis(ch); delay(1); }
    return (int)(s / n);
}
#endif

// Centered axis -> -500..+500 with deadzone and per-side scaling.
static int16_t scaleCentered(int v, int center, int invert) {
    int d = v - center;
    if (abs(d) <= ADC_DEADZONE) return 0;
    int out;
    if (d > 0) {
        int span = 4095 - ADC_EDGE_MARGIN - center - ADC_DEADZONE;
        out = (span > 0) ? (int)((long)(d - ADC_DEADZONE) * 500 / span) : 0;
    } else {
        int span = center - ADC_EDGE_MARGIN - ADC_DEADZONE;
        out = (span > 0) ? (int)((long)(d + ADC_DEADZONE) * 500 / span) : 0;
    }
    return clamp16(out * invert, -500, 500);
}

static uint16_t scaleThrottle(int v) {
    int lo = adcThrMin + ADC_DEADZONE;
    int hi = 4095 - ADC_EDGE_MARGIN;
    if (v <= lo) return 0;
    long out = (long)(v - lo) * 1000 / (hi - lo);
    return (uint16_t)(out > 1000 ? 1000 : out);
}

// ---- ESP-NOW ----------------------------------------------------------------
static void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len != (int)sizeof(NeuroFlyTelemetryPacket)) return;
    NeuroFlyTelemetryPacket t;
    memcpy(&t, data, sizeof(t));
    if (t.magic != NEUROFLY_MAGIC_BYTE || t.version != NEUROFLY_PROTOCOL_VERSION) return;
    if (t.checksum != neurofly_checksum(&t, sizeof(t))) return;
    bool pairAck = (t.flags & (1 << TLM_FLAG_PAIRING)) != 0;
    if (pairingMode) {
        if (!pairAck) return;   // only a pair-ack telemetry can pair us
    } else if (memcmp(info->src_addr, droneMac, 6) != 0) {
        return;                 // wrong sender
    }

    portENTER_CRITICAL(&rxMux);
    rxTelemetry = t;
    { uint32_t m = millis(); lastRxMs = m ? m : 1; }
    lastRxRssi = info->rx_ctrl ? info->rx_ctrl->rssi : t.rssi;
    if (pairingMode && !pairFound) {
        memcpy(pairMac, info->src_addr, 6);
        pairFound = true;
    }
    portEXIT_CRITICAL(&rxMux);
}

static bool addEncryptedPeer(const uint8_t *mac) {
    esp_now_del_peer(mac);
    esp_now_peer_info_t p = {};
    memcpy(p.peer_addr, mac, 6);
    p.channel = 0;
    p.ifidx = WIFI_IF_STA;
    p.encrypt = true;
    memcpy(p.lmk, NEUROFLY_LMK, 16);
    return esp_now_add_peer(&p) == ESP_OK;
}

static bool addBroadcastPeer() {
    esp_now_peer_info_t p = {};
    memcpy(p.peer_addr, BROADCAST_MAC, 6);
    p.channel = 0;
    p.ifidx = WIFI_IF_STA;
    p.encrypt = false;   // broadcast frames cannot be encrypted
    return esp_now_add_peer(&p) == ESP_OK;
}

static bool initEspNow() {
    esp_now_deinit();    // harmless if not initialised; makes retry clean
    if (esp_now_init() != ESP_OK) return false;
    if (esp_now_set_pmk((const uint8_t *)NEUROFLY_PMK) != ESP_OK) return false;
    esp_now_register_recv_cb(onDataRecv);
    bool ok = pairingMode ? addBroadcastPeer() : addEncryptedPeer(droneMac);
    return ok;
}

static const uint8_t *txTarget() { return pairingMode ? BROADCAST_MAC : droneMac; }

static void sendPacket() {
    if (!espOk) return;
    esp_now_send(txTarget(), (uint8_t *)&txPacket, sizeof(txPacket));
}

static void buildPacket(uint32_t now, bool forceDisarm) {
    bool armed = isArmed && !forceDisarm && !pairingMode;
    uint16_t thr = armed ? rawThrottle : 0;
    int roll = armed ? stickRoll : 0, pitch = armed ? stickPitch : 0, yaw = armed ? stickYaw : 0;

    // Vision corrections: AI_ASSIST + armed + fresh only
    if (armed && selectedMode == MODE_AI_ASSIST && (now - visionStamp) <= VISION_TIMEOUT_MS && visionStamp != 0) {
        roll  += clamp16(visionRoll,  -VISION_CLAMP, VISION_CLAMP);
        pitch += clamp16(visionPitch, -VISION_CLAMP, VISION_CLAMP);
    }

    txPacket.magic = NEUROFLY_MAGIC_BYTE;
    txPacket.version = NEUROFLY_PROTOCOL_VERSION;
    txPacket.mode = armed ? selectedMode : MODE_DISARMED;
    txPacket.throttle = thr;
    txPacket.roll = clamp16(roll, -500, 500);
    txPacket.pitch = clamp16(pitch, -500, 500);
    txPacket.yaw = clamp16(yaw, -500, 500);
    txPacket.aux_buttons = (armed ? (1 << AUX_BIT_ARM) : 0)
                         | (btnMode.pressed ? (1 << AUX_BIT_MODE) : 0)
                         | ((emergencyStart != 0 && (now - emergencyStart) < EMERGENCY_LATCH_MS) ? (1 << AUX_BIT_EMERGENCY) : 0);
    txPacket.packet_id = packetCounter++;
    txPacket.checksum = neurofly_checksum(&txPacket, sizeof(txPacket));
}

// ---- Inputs -------------------------------------------------------------------
static void readSticks() {
#if INPUT_MODE == INPUT_ANALOG_GIMBALS
    if (!adsOk) { rawThrottle = 0; stickRoll = stickPitch = stickYaw = 0; return; }  // fail safe
    rawThrottle = scaleThrottle(readAxis(ADS_CH_THROTTLE));
    stickRoll  = scaleCentered(readAxis(ADS_CH_ROLL),  adcCenterRoll,  INVERT_ROLL);
    stickPitch = scaleCentered(readAxis(ADS_CH_PITCH), adcCenterPitch, INVERT_PITCH);
    stickYaw   = scaleCentered(readAxis(ADS_CH_YAW),   adcCenterYaw,   INVERT_YAW);
#else
    uint32_t now = millis();
    btnD0.update(now); btnD1.update(now); btnD2.update(now); btnD3.update(now);
    bool shift = btnMode.pressed;
    if (shift && (btnD0.pressed || btnD1.pressed || btnD2.pressed || btnD3.pressed)) modeShiftUsed = true;

    int yaw = 0, roll = 0, pitch = 0;
    if (shift) {
        if (btnD0.pressed) yaw += BTN_DEFLECT;
        if (btnD1.pressed) yaw -= BTN_DEFLECT;
        if (btnD2.pressed) roll += BTN_DEFLECT;
        if (btnD3.pressed) roll -= BTN_DEFLECT;
    } else {
        if (btnD2.pressed) pitch += BTN_DEFLECT;
        if (btnD3.pressed) pitch -= BTN_DEFLECT;
    }
    stickYaw = yaw; stickRoll = roll; stickPitch = pitch;
    // throttle ramps are applied once per TX tick in loop()
#endif
}

static void handleArmAndMode(uint32_t now) {
    btnArm.update(now);
    btnMode.update(now);

    // --- ARM / EMERGENCY ---
    if (btnArm.fell) armLongHandled = false;
    if (btnArm.pressed && !armLongHandled && (now - btnArm.pressedAt) >= EMERGENCY_HOLD_MS) {
        armLongHandled = true;
        isArmed = false;
        rawThrottle = 0;
        emergencyStart = now ? now : 1;
    }
    if (btnArm.rose) {
        if (!armLongHandled) {
            if (isArmed) {
                isArmed = false;
                rawThrottle = 0;
            } else if (!pairingMode && rawThrottle <= ARM_MAX_THROTTLE) {
                if (selectedMode > MODE_ALT_HOLD) selectedMode = MODE_ARMED_MANUAL;
                isArmed = true;
            }
            // else: refused (throttle not at minimum / pairing)
        }
        armLongHandled = false;
    }

    // --- MODE ---
    if (btnMode.fell) modeShiftUsed = false;
    if (btnMode.rose) {
        bool shortPress = (now - btnMode.pressedAt) < MODE_SHORT_PRESS_MS;
        if (!modeIgnoreRelease && !modeShiftUsed && shortPress) {
            if (!isArmed) {
                selectedMode = (selectedMode >= MODE_AI_ASSIST) ? MODE_ARMED_MANUAL : selectedMode + 1;
            } else if (selectedMode == MODE_ARMED_MANUAL) {
                selectedMode = MODE_ALT_HOLD;
            } else {
                selectedMode = MODE_ARMED_MANUAL;   // AltHold/Auto/AI -> Manual
            }
        }
        modeIgnoreRelease = false;
    }
}

// Accept lines like {"vr":12,"vp":-30} from the PC. Never blocks.
static void pollSerialInput(uint32_t now) {
    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (serialOverflow) { serialOverflow = false; serialLen = 0; continue; }
            serialBuf[serialLen] = 0;
            const char *r = strstr(serialBuf, "\"vr\"");
            const char *p = strstr(serialBuf, "\"vp\"");
            if (r && p) {
                r = strchr(r, ':'); p = strchr(p, ':');
                if (r && p) {
                    visionRoll = clamp16(atoi(r + 1), -VISION_CLAMP, VISION_CLAMP);
                    visionPitch = clamp16(atoi(p + 1), -VISION_CLAMP, VISION_CLAMP);
                    visionStamp = now ? now : 1;
                }
            }
            serialLen = 0;
        } else if (serialLen < sizeof(serialBuf) - 1) {
            serialBuf[serialLen++] = c;
        } else {
            serialLen = 0; serialOverflow = true;   // overflow: drop the whole line
        }
    }
}

// ---- OLED -----------------------------------------------------------------------
static void drawOled(uint32_t now, bool link, uint16_t bat, int rssi) {
    if (!oledOk) return;
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(modeName(txPacket.mode == MODE_DISARMED ? selectedMode : txPacket.mode));
    display.setCursor(80, 0);
    display.print(isArmed ? "ARMED" : "SAFE");

    display.setCursor(0, 14);
    display.print("THR");
    display.drawRect(24, 14, 100, 8, SSD1306_WHITE);
    display.fillRect(24, 14, txPacket.throttle / 10, 8, SSD1306_WHITE);

    display.setCursor(0, 28);
    if (link) display.printf("BAT %u.%02uV", bat / 1000, (bat % 1000) / 10);
    else display.print("BAT --");
    display.setCursor(80, 28);
    if (link) display.printf("%ddBm", rssi); else display.print("-- dBm");

    display.setCursor(0, 42);
    display.printf("R%d P%d Y%d", txPacket.roll, txPacket.pitch, txPacket.yaw);

    display.setCursor(0, 54);
    if (!espOk) display.print("ESP-NOW ERROR");
    else if (pairingMode) display.print("PAIRING...");
    else if (!link) display.print("! LINK LOST !");
    else display.print("LINK OK");
    display.display();
}

// ---- Setup / loop -----------------------------------------------------------
void setup() {
    Serial.begin(115200);
#if defined(ARDUINO_USB_CDC_ON_BOOT) && ARDUINO_USB_CDC_ON_BOOT
    Serial.setTxTimeoutMs(0);   // never stall the 40 Hz loop if no PC is attached
#endif
    delay(200);
    Serial.printf("NeuroFly controller firmware %s (protocol v%d, input=%s)\n",
                  NEUROFLY_FIRMWARE_VERSION, NEUROFLY_PROTOCOL_VERSION,
                  INPUT_MODE == INPUT_ANALOG_GIMBALS ? "analog" : "buttons");

    Wire.begin();   // XIAO C6 default: SDA=D4, SCL=D5 (OLED + optional ADS1115)
    Wire.setClock(400000);

    btnArm.begin(PIN_ARM);
    btnMode.begin(PIN_MODE);
#if INPUT_MODE == INPUT_BUTTONS
    btnD0.begin(PIN_THROTTLE); btnD1.begin(PIN_YAW);
    btnD2.begin(PIN_PITCH);    btnD3.begin(PIN_ROLL);
#else
    adsOk = ads.begin(ADS_ADDR, &Wire);
    if (adsOk) {
        ads.setGain(GAIN_ONE);
        ads.setDataRate(RATE_ADS1115_860SPS);
        // Startup calibration: throttle at bottom, other sticks centred
        int t = readAdcAvg(ADS_CH_THROTTLE);
        adcThrMin = (t < 1200) ? t : 150;   // throttle not at bottom -> safe fallback
        adcCenterRoll = readAdcAvg(ADS_CH_ROLL);
        adcCenterPitch = readAdcAvg(ADS_CH_PITCH);
        adcCenterYaw = readAdcAvg(ADS_CH_YAW);
    } else {
        Serial.println("{\"error\":\"ADS1115 not found at 0x48: sticks disabled\"}");
    }
#endif

    // Hold MODE at boot => pairing mode. No stored drone also forces pairing.
    delay(50);
    bool modeHeld = (digitalRead(PIN_MODE) == LOW);
    prefs.begin("neurofly", false);
    hasPairedMac = (prefs.getBytes("drone", droneMac, 6) == 6);
    pairingMode = modeHeld || !hasPairedMac;
    if (modeHeld) modeIgnoreRelease = true;   // releasing the button must not change mode
    Serial.println(pairingMode ? "Pairing mode: broadcasting unencrypted DISARMED requests" : "Paired mode: encrypted unicast");

    // OLED (optional)
    Wire.begin();   // XIAO C6 default: SDA=D4, SCL=D5
    Wire.setClock(400000);
    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() == 0) {
        oledOk = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    }
    if (!oledOk) Serial.println("OLED not found; continuing without display");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    espOk = initEspNow();
    if (!espOk) Serial.println("{\"error\":\"ESP-NOW Init Failed, retrying\"}");

    // A few DISARM packets so the drone starts from a known safe state
    isArmed = false;
    for (int i = 0; i < 5; i++) {
        buildPacket(millis(), true);
        sendPacket();
        delay(10);
    }
}

void loop() {
    uint32_t now = millis();

    // Retry ESP-NOW init once per second if it failed
    if (!espOk && now - lastEspRetry >= 1000) {
        lastEspRetry = now;
        espOk = initEspNow();
        Serial.println(espOk ? "{\"info\":\"ESP-NOW recovered\"}" : "{\"error\":\"ESP-NOW Init Failed, retrying\"}");
    }

    // Pairing complete: persist MAC, swap broadcast peer for encrypted peer
    if (pairFound) {
        memcpy(droneMac, pairMac, 6);
        prefs.putBytes("drone", droneMac, 6);
        hasPairedMac = true;
        pairingMode = false;
        pairFound = false;
        esp_now_del_peer(BROADCAST_MAC);
        espOk = addEncryptedPeer(droneMac);
        Serial.printf("{\"info\":\"paired\",\"mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\"}\n",
                      droneMac[0], droneMac[1], droneMac[2], droneMac[3], droneMac[4], droneMac[5]);
    }

    handleArmAndMode(now);
    readSticks();
    pollSerialInput(now);

    // Fixed-rate TX schedule (catches up, so OLED redraws do not lower the average rate)
    static uint32_t nextTx = 0;
    static uint32_t nextOled = 0;
    if ((int32_t)(now - nextTx) < 0) return;
    nextTx += TX_PERIOD_MS;
    if ((int32_t)(now - nextTx) > 100) nextTx = now + TX_PERIOD_MS;   // resync after a long stall

#if INPUT_MODE == INPUT_BUTTONS
    if (isArmed && !btnMode.pressed) {
        if (btnD0.pressed && rawThrottle < 1000) rawThrottle = min(1000, rawThrottle + BTN_THROTTLE_STEP);
        if (btnD1.pressed && rawThrottle > 0)    rawThrottle = (rawThrottle > BTN_THROTTLE_STEP) ? rawThrottle - BTN_THROTTLE_STEP : 0;
    } else if (!isArmed) {
        rawThrottle = 0;
    }
#endif

    buildPacket(now, false);
    sendPacket();

    // Snapshot telemetry
    bool link = linkOk(now);
    NeuroFlyTelemetryPacket t;
    int rssi;
    portENTER_CRITICAL(&rxMux);
    t = rxTelemetry;
    rssi = lastRxRssi;
    portEXIT_CRITICAL(&rxMux);
    uint16_t bat = link ? t.battery_mv : 0;
    if (!link) rssi = 0;

    // JSON for the web simulator, e.g.
    // {"armed":1,"thr":450,"r":0,"p":120,"y":0,"mode":1,"bat":3700,"rssi":-50,"ver":"0.3.0"}
    Serial.printf("{\"armed\":%d,\"thr\":%d,\"r\":%d,\"p\":%d,\"y\":%d,\"mode\":%d,\"bat\":%u,\"rssi\":%d,\"ver\":\"%s\"}\n",
                  (txPacket.aux_buttons & (1 << AUX_BIT_ARM)) ? 1 : 0,
                  (int)txPacket.throttle, (int)txPacket.roll, (int)txPacket.pitch, (int)txPacket.yaw,
                  (int)txPacket.mode, (unsigned)bat, rssi, NEUROFLY_FIRMWARE_VERSION);

    // OLED at ~5 Hz (one frame ~25 ms at 400 kHz; the TX schedule catches up)
    if (oledOk && (int32_t)(now - nextOled) >= 0) {
        nextOled = now + OLED_PERIOD_MS;
        drawOled(now, link, bat, rssi);
    }
}
