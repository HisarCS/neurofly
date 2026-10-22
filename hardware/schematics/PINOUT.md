# NeuroFly Hardware & Pinout Specifications

This document defines the pinout mapping, peripheral assignments, and circuit design guidelines for the Seeed Studio XIAO ESP32-C6 when used on both the **NeuroFly Drone** and the **NeuroFly Remote Controller**.

---

## 1. Seeed Studio XIAO ESP32-C6 Pin Mapping

| Pin Label | ESP32-C6 GPIO | Default Peripherals |
| :--- | :--- | :--- |
| **D0** | GPIO0 | ADC1_CH0, LP_GPIO0 |
| **D1** | GPIO1 | ADC1_CH1, LP_GPIO1 |
| **D2** | GPIO2 | ADC1_CH2 |
| **D3** | GPIO21 | PWM, General IO |
| **D4** | GPIO22 | I2C SDA (Default Hardware I2C) |
| **D5** | GPIO23 | I2C SCL (Default Hardware I2C) |
| **D6** | GPIO16 | TX (UART) / PWM / General IO |
| **D7** | GPIO17 | RX (UART) / PWM / General IO |
| **D8** | GPIO18 | PWM, General IO |
| **D9** | GPIO19 | PWM, General IO |
| **D10** | GPIO20 | PWM, General IO |

---

## 2. NeuroFly Drone (Flight Controller Carrier Board)

The XIAO ESP32-C6 mounts directly onto a custom KiCad carrier PCB via female headers or castellated SMD solder pads.

| Function | XIAO Pin | ESP32-C6 GPIO | Description |
| :--- | :--- | :--- | :--- |
| **Motor 1 (Front Left - CW)** | D0 | GPIO0 | PWM Output -> N-FET Gate (AO3400) |
| **Motor 2 (Front Right - CCW)** | D1 | GPIO1 | PWM Output -> N-FET Gate (AO3400) |
| **Motor 3 (Rear Right - CW)** | D2 | GPIO2 | PWM Output -> N-FET Gate (AO3400) |
| **Motor 4 (Rear Left - CCW)** | D3 | GPIO21 | PWM Output -> N-FET Gate (AO3400) |
| **I2C SDA** | D4 | GPIO22 | Shared I2C SDA for MPU6050 (IMU) & VL53L0X (ToF) |
| **I2C SCL** | D5 | GPIO23 | Shared I2C SCL for MPU6050 (IMU) & VL53L0X (ToF) |
| **Battery Voltage Monitor** | D6 | GPIO16 | 1S LiPo Voltage Divider (2x 10kΩ) -> ADC Input |
| **Status LED / Buzzer** | D7 | GPIO17 | High-visibility status indicator / Piezo buzzer |
| **ToF XSHUT (Optional)** | D8 | GPIO18 | Hardware shutdown pin for ToF sensor sleep |

### Motor Drive Topology (Per Motor Channel):
```text
                  +3.7V (1S LiPo)
                       │
                     ┌─┴─────────┐
                     │           │
                 [Motor]      [1N5819 Schottky]
                     │        (Cathode to VCC)
                     │           │
                     └─┬─────────┘
                       │
                    Drain (D)
GPIO PWM ──[100Ω]───Gate (G) [AO3400 N-MOSFET]
                       │
                  Source (S)
                       │
                      GND
                       │
       (10kΩ Pull-down resistor between Gate and GND)
```

---

## 3. NeuroFly Remote Controller (Transmitter Board)

The handheld controller functions both as a standalone RF transmitter (ESP-NOW) and as a USB-C game controller for the browser flight simulator (Web Serial API).

| Peripheral | XIAO Pin | ESP32-C6 GPIO | Description |
| :--- | :--- | :--- | :--- |
| **OLED SDA** | D4 | GPIO22 | 0.96" SSD1306 OLED Display (I2C SDA) |
| **OLED SCL** | D5 | GPIO23 | 0.96" SSD1306 OLED Display (I2C SCL) |
| **Throttle Input** | D0 | GPIO0 | Up/Down push buttons or Left Gimbal Y-Axis |
| **Yaw Input** | D1 | GPIO1 | Left/Right push buttons or Left Gimbal X-Axis |
| **Pitch Input** | D2 | GPIO2 | Forward/Backward buttons or Right Gimbal Y-Axis |
| **Roll Input** | D3 | GPIO21 | Left/Right banking buttons or Right Gimbal X-Axis |
| **Arm / Disarm Button** | D8 | GPIO18 | Safety arm toggle switch / pushbutton |
| **Flight Mode Switch** | D9 | GPIO19 | Cycle: Manual / Alt-Hold / Autonomous |

---

## 4. KiCad Fabrication Guidelines (FabLab & Fab Academy Standards)

1. **Dual Fabrication Compatibility:**
   - **Milled PCBs (FabLab Roland SRM-20 / Modela):**
     - Single-sided or double-sided routing with top ground plane.
     - Minimum trace width: 0.40 mm (16 mil).
     - Minimum clearance: 0.40 mm (16 mil).
   - **Commercial Fabrication (JLCPCB / PCBWay):**
     - Standard 1.2mm or 0.8mm FR4 thickness to save weight.
     - 0805 or 1206 passive footprint sizing for easy hand-soldering by students.
2. **Weight Optimization:**
   - Structural carrier board includes internal cutouts (honeycomb/cross lattice) to reduce mass while preserving stiffness.
