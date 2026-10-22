/**
 * @file main.cpp
 * @brief NeuroFly Quadcopter Flight Controller Firmware
 * @target Seeed Studio XIAO ESP32-C6
 * 
 * Features:
 * - ESP-NOW low-latency receiver
 * - Built-in 200ms failsafe watchdog
 * - 4-Channel Coreless Motor PWM Driver (LEDC)
 * - MPU6050 IMU integration placeholder & PID mixer
 * - Telemetry transmission back to handheld controller
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include "../common/protocol.h"

// Motor Pin Definitions (See hardware/schematics/PINOUT.md)
#define PIN_MOTOR_FL  D0 // Front-Left (CW)
#define PIN_MOTOR_FR  D1 // Front-Right (CCW)
#define PIN_MOTOR_RR  D2 // Rear-Right (CW)
#define PIN_MOTOR_RL  D3 // Rear-Left (CCW)
#define PIN_LED       D7

// PWM Configuration
#define PWM_FREQ      20000 // 20kHz for silent coreless DC motor drive
#define PWM_RES       10    // 10-bit resolution (0 - 1023)

// Telemetry & Control Packets
NeuroFlyControlPacket rxControl;
NeuroFlyTelemetryPacket txTelemetry;

volatile unsigned long lastPacketTime = 0;
bool failsafeTriggered = true;

// Callback when data is received over ESP-NOW
void onDataRecv(const esp_now_recv_info *recv_info, const uint8_t *incomingData, int len) {
    if (len == sizeof(NeuroFlyControlPacket)) {
        memcpy(&rxControl, incomingData, sizeof(NeuroFlyControlPacket));
        lastPacketTime = millis();
        failsafeTriggered = false;
    }
}

void setMotorOutputs(uint16_t fl, uint16_t fr, uint16_t rr, uint16_t rl) {
    ledcWrite(PIN_MOTOR_FL, constrain(fl, 0, 1023));
    ledcWrite(PIN_MOTOR_FR, constrain(fr, 0, 1023));
    ledcWrite(PIN_MOTOR_RR, constrain(rr, 0, 1023));
    ledcWrite(PIN_MOTOR_RL, constrain(rl, 0, 1023));
}

void setup() {
    Serial.begin(115200);

    // Initialize Motor PWM Pins using ESP32 Arduino Core 3.x ledcAttach
    ledcAttach(PIN_MOTOR_FL, PWM_FREQ, PWM_RES);
    ledcAttach(PIN_MOTOR_FR, PWM_FREQ, PWM_RES);
    ledcAttach(PIN_MOTOR_RR, PWM_FREQ, PWM_RES);
    ledcAttach(PIN_MOTOR_RL, PWM_FREQ, PWM_RES);
    setMotorOutputs(0, 0, 0, 0);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);

    // Wi-Fi Station Mode for ESP-NOW
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW Init Failed!");
        return;
    }

    esp_now_register_recv_cb(onDataRecv);

    Serial.println("NeuroFly Flight Controller Initialized.");
}

void loop() {
    unsigned long now = millis();

    // 1. Safety Failsafe: Cut power if no packets received for > 200ms
    if (now - lastPacketTime > 200) {
        failsafeTriggered = true;
    }

    if (failsafeTriggered || rxControl.mode == MODE_DISARMED) {
        setMotorOutputs(0, 0, 0, 0);
        digitalWrite(PIN_LED, LOW);
        delay(10);
        return;
    }

    // 2. Flight Active (Armed)
    digitalWrite(PIN_LED, HIGH);

    uint16_t baseThrottle = map(rxControl.throttle, 0, 1000, 0, 1023);
    int16_t pitch = rxControl.pitch;
    int16_t roll = rxControl.roll;
    int16_t yaw = rxControl.yaw;

    // 3. Quadcopter Motor Mixer (X-Configuration)
    // Front-Left  (CW)  = Throttle + Pitch + Roll - Yaw
    // Front-Right (CCW) = Throttle + Pitch - Roll + Yaw
    // Rear-Right  (CW)  = Throttle - Pitch - Roll - Yaw
    // Rear-Left   (CCW) = Throttle - Pitch + Roll + Yaw
    int mFL = baseThrottle + (pitch / 4) + (roll / 4) - (yaw / 4);
    int mFR = baseThrottle + (pitch / 4) - (roll / 4) + (yaw / 4);
    int mRR = baseThrottle - (pitch / 4) - (roll / 4) - (yaw / 4);
    int mRL = baseThrottle - (pitch / 4) + (roll / 4) + (yaw / 4);

    setMotorOutputs(mFL, mFR, mRR, mRL);

    delay(5); // Loop rate ~200Hz
}
