#pragma once

#include <stdint.h>

// NeuroFly ESP-NOW Haberleşme Protokolü Tanımları

#define NEUROFLY_MAGIC_BYTE 0x4E // 'N'
#define NEUROFLY_PROTOCOL_VERSION 1

// Kontrol Modları
enum FlightMode : uint8_t {
    MODE_DISARMED = 0,
    MODE_ARMED_MANUAL = 1,
    MODE_ALT_HOLD = 2,      // Otonom İrtifa Sabitleme
    MODE_AUTONOMOUS = 3,    // Otonom Parkur / Görev
    MODE_AI_ASSIST = 4      // AI Destekli Uçuş
};

// Kumandadan Drone'a Gönderilen Paket (Transmitter -> Drone)
// Gönderim Sıklığı: 20Hz - 50Hz (20ms - 50ms)
#pragma pack(push, 1)
struct NeuroFlyControlPacket {
    uint8_t magic;          // NEUROFLY_MAGIC_BYTE
    uint8_t version;        // NEUROFLY_PROTOCOL_VERSION
    uint8_t mode;           // FlightMode enum
    
    // Değer aralığı: 0 - 255 (veya -127 ila +127)
    uint16_t throttle;      // 0 - 1000 (Gaz seviyesi)
    int16_t roll;           // -500 ila +500 (Sağ/Sol yatış)
    int16_t pitch;          // -500 ila +500 (İleri/Geri eğilme)
    int16_t yaw;            // -500 ila +500 (Kendi ekseninde dönüş)
    
    uint8_t aux_buttons;    // Bitmask: [0]=Arm/Disarm, [1]=ModeToggle, [2]=EmergencyStop, [3..7]=Opsiyonel
    uint32_t packet_id;     // Sıra numarası (paket kaybı tespiti için)
    uint8_t checksum;       // Basit XOR veya CRC8 kontrolü
};

// Drone'dan Kumandaya Gönderilen Telemetri Paketi (Drone -> Transmitter)
// Gönderim Sıklığı: 5Hz - 10Hz
struct NeuroFlyTelemetryPacket {
    uint8_t magic;
    uint8_t current_mode;
    uint16_t battery_mv;    // Batarya voltajı (örn. 3700 = 3.70V)
    int16_t pitch_angle;    // 0.1 derece hassasiyetle anlık açı
    int16_t roll_angle;     // 0.1 derece hassasiyetle anlık açı
    uint16_t altitude_mm;   // ToF sensör varsa mesafe (mm cinsinden)
    int8_t rssi;            // Sinyal gücü (dBm)
    uint8_t flags;          // [0]=Failsafe, [1]=LowBattery, [2]=IMU_Calibrated
};
#pragma pack(pop)
