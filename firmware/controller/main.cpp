/**
 * @file main.cpp
 * @brief NeuroFly Smart Remote Controller Firmware
 * @target Seeed Studio XIAO ESP32-C6
 * 
 * Features:
 * - Reads directional buttons / analog sticks
 * - Broadcasts low-latency control packets via ESP-NOW to the drone
 * - Streams telemetry / stick state via Serial (USB-CDC) to NeuroFly Web Simulator
 * - Updates 0.96" SSD1306 OLED display
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include "../common/protocol.h"

// Pin Definitions according to hardware/schematics/PINOUT.md
#define PIN_THROTTLE_UP    D0
#define PIN_THROTTLE_DOWN  D1
#define PIN_PITCH_UP       D2
#define PIN_PITCH_DOWN     D3
#define PIN_ARM_TOGGLE     D8
#define PIN_MODE_TOGGLE    D9

// Global Packets
NeuroFlyControlPacket txPacket;
NeuroFlyTelemetryPacket rxTelemetry;

// Drone Broadcast MAC (FF:FF:FF:FF:FF:FF for open pairing or targeted peer)
uint8_t droneBroadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

bool isArmed = false;
uint16_t currentThrottle = 0;
uint32_t packetCounter = 0;
unsigned long lastTxTime = 0;

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Optional transmission health diagnostics
}

void onDataRecv(const esp_now_recv_info *recv_info, const uint8_t *incomingData, int len) {
    if (len == sizeof(NeuroFlyTelemetryPacket)) {
        memcpy(&rxTelemetry, incomingData, sizeof(NeuroFlyTelemetryPacket));
    }
}

void setup() {
    Serial.begin(115200);
    
    // Configure input pins with internal pull-up
    pinMode(PIN_THROTTLE_UP, INPUT_PULLUP);
    pinMode(PIN_THROTTLE_DOWN, INPUT_PULLUP);
    pinMode(PIN_PITCH_UP, INPUT_PULLUP);
    pinMode(PIN_PITCH_DOWN, INPUT_PULLUP);
    pinMode(PIN_ARM_TOGGLE, INPUT_PULLUP);
    pinMode(PIN_MODE_TOGGLE, INPUT_PULLUP);

    // Initialize Wi-Fi in Station mode for ESP-NOW
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("{\"error\":\"ESP-NOW Init Failed\"}");
        return;
    }

    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataRecv);

    // Register peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, droneBroadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);

    // Initialize base packet
    txPacket.magic = NEUROFLY_MAGIC_BYTE;
    txPacket.version = NEUROFLY_PROTOCOL_VERSION;
    txPacket.mode = MODE_DISARMED;
}

void loop() {
    unsigned long now = millis();

    // Check Arm button toggle (Active LOW)
    static bool lastArmBtnState = HIGH;
    bool currentArmBtnState = digitalRead(PIN_ARM_TOGGLE);
    if (lastArmBtnState == HIGH && currentArmBtnState == LOW) {
        isArmed = !isArmed;
        if (!isArmed) currentThrottle = 0;
        delay(50); // Debounce
    }
    lastArmBtnState = currentArmBtnState;

    // Read Inputs (Example discrete button step adjustments)
    if (isArmed) {
        if (digitalRead(PIN_THROTTLE_UP) == LOW) {
            if (currentThrottle < 1000) currentThrottle += 5;
        }
        if (digitalRead(PIN_THROTTLE_DOWN) == LOW) {
            if (currentThrottle > 0) currentThrottle -= 5;
        }
    } else {
        currentThrottle = 0;
    }

    int16_t pitch = 0;
    if (digitalRead(PIN_PITCH_UP) == LOW) pitch = 250;
    if (digitalRead(PIN_PITCH_DOWN) == LOW) pitch = -250;

    // Send control packet at ~40Hz (25ms period)
    if (now - lastTxTime >= 25) {
        lastTxTime = now;

        txPacket.mode = isArmed ? MODE_ARMED_MANUAL : MODE_DISARMED;
        txPacket.throttle = currentThrottle;
        txPacket.pitch = pitch;
        txPacket.roll = 0;
        txPacket.yaw = 0;
        txPacket.packet_id = packetCounter++;

        // 1. Transmit over ESP-NOW to real drone
        esp_now_send(droneBroadcastAddress, (uint8_t *)&txPacket, sizeof(txPacket));

        // 2. Stream JSON to Web Serial for 3D Simulator
        // Output Format: {"armed":1,"thr":450,"r":0,"p":120,"y":0}
        Serial.printf("{\"armed\":%d,\"thr\":%d,\"r\":%d,\"p\":%d,\"y\":%d}\n",
                      isArmed ? 1 : 0,
                      txPacket.throttle,
                      txPacket.roll,
                      txPacket.pitch,
                      txPacket.yaw);
    }
}
