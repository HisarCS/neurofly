#pragma once

#include <stdint.h>

// NeuroFly ESP-NOW Wireless Telemetry & Control Protocol

#define NEUROFLY_MAGIC_BYTE 0x4E // 'N'
#define NEUROFLY_PROTOCOL_VERSION 1

// Flight Operating Modes
enum FlightMode : uint8_t {
    MODE_DISARMED = 0,
    MODE_ARMED_MANUAL = 1,
    MODE_ALT_HOLD = 2,      // Autonomous Altitude Hold (ToF / Baro)
    MODE_AUTONOMOUS = 3,    // Autonomous Course / Waypoint
    MODE_AI_ASSIST = 4      // AI / Vision Guided Navigation
};

// Control Packet: Transmitter -> Drone (Target Rate: 20Hz - 50Hz)
#pragma pack(push, 1)
struct NeuroFlyControlPacket {
    uint8_t magic;          // NEUROFLY_MAGIC_BYTE
    uint8_t version;        // NEUROFLY_PROTOCOL_VERSION
    uint8_t mode;           // FlightMode enum
    
    // Normalized values: Throttle: 0-1000, Roll/Pitch/Yaw: -500 to +500
    uint16_t throttle;      // 0 - 1000
    int16_t roll;           // -500 to +500
    int16_t pitch;          // -500 to +500
    int16_t yaw;            // -500 to +500
    
    uint8_t aux_buttons;    // Bitmask: [0]=Arm/Disarm, [1]=ModeToggle, [2]=EmergencyCut, [3..7]=Aux
    uint32_t packet_id;     // Monotonic sequence number for packet loss calculation
    uint8_t checksum;       // Simple XOR or CRC-8 checksum
};

// Telemetry Packet: Drone -> Transmitter (Target Rate: 5Hz - 10Hz)
struct NeuroFlyTelemetryPacket {
    uint8_t magic;
    uint8_t current_mode;
    uint16_t battery_mv;    // Battery voltage in millivolts (e.g. 3700 = 3.70V)
    int16_t pitch_angle;    // Pitch angle in 0.1 degree units
    int16_t roll_angle;     // Roll angle in 0.1 degree units
    uint16_t altitude_mm;   // ToF laser distance in millimeters
    int8_t rssi;            // Signal strength in dBm
    uint8_t flags;          // Bitmask: [0]=Failsafe, [1]=LowBattery, [2]=IMU_Calibrated
};
#pragma pack(pop)
