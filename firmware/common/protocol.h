#pragma once

#include <stdint.h>
#include <stddef.h>

// NeuroFly ESP-NOW Wireless Telemetry & Control Protocol

#define NEUROFLY_MAGIC_BYTE 0x4E // 'N'
#define NEUROFLY_PROTOCOL_VERSION 2   // v2: checksum enforced, telemetry has version+checksum, pairing key
#define NEUROFLY_FIRMWARE_VERSION "0.3.0"

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
    uint8_t checksum;       // XOR of every preceding byte of the packet (see neurofly_checksum)
};

// Telemetry Packet: Drone -> Transmitter (Target Rate: 5Hz - 10Hz)
struct NeuroFlyTelemetryPacket {
    uint8_t magic;
    uint8_t version;        // NEUROFLY_PROTOCOL_VERSION
    uint8_t current_mode;
    uint16_t battery_mv;    // Battery voltage in millivolts (e.g. 3700 = 3.70V)
    int16_t pitch_angle;    // Pitch angle in 0.1 degree units
    int16_t roll_angle;     // Roll angle in 0.1 degree units
    uint16_t altitude_mm;   // ToF laser distance in millimeters
    int8_t rssi;            // Signal strength in dBm
    uint8_t flags;          // Bitmask: [0]=Failsafe, [1]=LowBattery, [2]=IMU_Calibrated, [7]=PairingAck
    uint8_t checksum;       // XOR of every preceding byte of the packet
};
#pragma pack(pop)

// aux_buttons bit positions
#define AUX_BIT_ARM        0
#define AUX_BIT_MODE       1
#define AUX_BIT_EMERGENCY  2

// Telemetry flags bit positions
#define TLM_FLAG_FAILSAFE    0
#define TLM_FLAG_LOW_BATT    1
#define TLM_FLAG_IMU_CALIB   2
#define TLM_FLAG_PAIRING     7   // pair-ack: sent UNENCRYPTED by an unpaired drone in reply to a pairing request

// XOR checksum over the first (len - 1) bytes; the last byte of the packet holds the result.
static inline uint8_t neurofly_checksum(const void *pkt, size_t len) {
    const uint8_t *p = (const uint8_t *)pkt;
    uint8_t c = 0;
    for (size_t i = 0; i + 1 < len; i++) c ^= p[i];
    return c;
}

// Pairing / link security (ESP-NOW encrypted frames can only be received from a peer that is
// registered with the same LMK, so the handshake has to start unencrypted):
//  * Both sides share the 16-byte PMK/LMK below. CHANGE PER CLASSROOM / TEAM before flashing.
//  * PAIRED: each side has the other's MAC in NVS and registers it as an ENCRYPTED peer at boot;
//    frames from any other MAC are ignored. Armed/flight frames are only ever accepted this way.
//  * PAIRING (controller: MODE held at boot or nothing stored; drone: nothing stored in NVS):
//      controller -> broadcast, UNENCRYPTED, DISARMED control frames (valid magic/version/checksum)
//      drone      -> accepts ONLY such DISARMED zero-throttle frames, replies UNENCRYPTED unicast
//                    telemetry with TLM_FLAG_PAIRING set (pair-ack), stores the MAC, then swaps to an
//                    encrypted peer. Motors stay off in the pairing state.
//      controller -> on a valid telemetry with TLM_FLAG_PAIRING stores the drone MAC and swaps to
//                    encrypted unicast.
//  * Clear pairing: drone = hold BOOT (GPIO9) 3 s within 5 s after start-up; controller = hold MODE at boot.
//  * Status: compile-verified only; not yet bench-tested on hardware.
#define NEUROFLY_PMK "NeuroFlyPMK-0001"   // 16 chars
#define NEUROFLY_LMK "NeuroFlyLMK-0001"   // 16 chars
