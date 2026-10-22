# NeuroFly - Bill of Materials (BOM)

This document provides a comprehensive component breakdown for the **NeuroFly Drone** and the **NeuroFly Smart Remote Controller**, including digital fabrication materials compatible with standard FabLab / makerspace equipment.

---

## 1. NeuroFly Drone (Flight Platform)

| Component | Qty | Part / Specifications | FabLab Notes |
| :--- | :---: | :--- | :--- |
| **Microcontroller** | 1 | Seeed Studio XIAO ESP32-C6 | Onboard ceramic antenna or external U.FL |
| **IMU Sensor** | 1 | MPU6050 or LSM6DS3 breakout | 6-DOF Gyroscope + Accelerometer (I2C) |
| **Ranging Sensor (Optional)** | 1 | VL53L0X or VL53L1X | ToF sensor: **required only** for altitude-hold and the +50/+75 autonomous missions; the drone flies manually without it |
| **Motors** | 4 | 716 (7x16mm) or 8520 Coreless DC | 2x CW (Clockwise), 2x CCW (Counter-Clockwise) |
| **Propellers** | 4 | 31mm - 55mm Micro Propellers | 2x CW, 2x CCW |
| **Motor Drivers** | 4 | Logic-Level N-MOSFET (AO3400 / SI2302) | SOT-23 packages with 3.3V logic gate saturation |
| **Flyback Diodes** | 4 | Schottky Diode (1N5819 or SS14) | Absorbs motor inductive back-EMF spikes |
| **Gate Resistors** | 4 | 100Ω (gate series, one per motor) | 0805 SMD / through-hole |
| **Gate Pull-Down Resistors** | 4 | 10kΩ (gate to GND, one per motor) | 0805 SMD / through-hole |
| **Battery Divider Resistors** | 2 | 10kΩ 1% (Vbat/2 to ADC; 4.2 V -> 2.1 V) | 0805 SMD, see PINOUT.md |
| **Bulk Capacitor** | 1 | 10µF low-ESR ceramic (battery pads decoupling) | 0805 / 1206, 6.3 V or higher |
| **ADC Filter Capacitor (optional)** | 1 | 100nF (ADC node to GND) | 0805 |
| **Status LED / Buzzer** | 1 | LED + 330Ω, or small piezo buzzer (D7) | Optional; needed for the "ready" indication in the checklist |
| **Headers** | 2 | 7-pin 2.54mm female (XIAO) + 4-pin I2C header | |
| **Battery** | 1 | 1S 3.7V 300mAh - 450mAh LiPo (25C-30C) | JST-PH 2.0 or BT2.0 connector |
| **Main Chassis** | 1 | 2mm / 3mm Cast Acrylic or Baltic Birch | **Laser Cut** (FabLab CO2 Laser) |
| **Prop Guards & Ducts** | 1-4 | Lightweight TPU / PLA | **3D Printed** (FDM, 0.2mm layer height) |

---

## 2. NeuroFly Remote Controller (Transmitter)

| Component | Qty | Part / Specifications | FabLab Notes |
| :--- | :---: | :--- | :--- |
| **Microcontroller** | 1 | Seeed Studio XIAO ESP32-C6 | Dual-mode: ESP-NOW RF link & USB-C Web Serial |
| **Display** | 1 | 0.96" I2C Monochrome OLED (SSD1306, 128x64) | Shows battery, RSSI, telemetry, mode |
| **Inputs (option A: gimbals, build with `-DINPUT_MODE=1`)** | 2 | Mini analog thumbsticks, 10kΩ pots, powered from 3V3 | Throttle/Yaw (left), Pitch/Roll (right). Needs the ADS1115 below: the ESP32-C6 ADC only exists on GPIO0-6 |
| **ADS1115 16-bit I2C ADC module (option A only, optional)** | 1 | ADS1115 breakout, I2C 0x48 (ADDR to GND), on the OLED I2C bus (D4/D5) | Reads the 4 gimbal axes: A0 throttle, A1 yaw, A2 pitch, A3 roll. Not needed for the button variant |
| **Inputs (option B: buttons, default)** | 4 | 12x12mm Tactile Buttons on D0-D3 | Throttle up/down, pitch +/-; MODE held acts as shift for yaw/roll; internal pull-ups, no resistors |
| **Arm & Mode Switches** | 2 | Tactile button or toggle switch (to GND) | D8 = Arm, D9 = Mode |
| **Emergency Cut** | 0 | No extra part: hold the ARM button 1 s | See `docs/competition/RULES.md` |
| **Gimbal Noise Capacitors (optional)** | 4 | 100nF wiper-to-GND | Option A only |
| **Battery & Charging** | 1 | 1S 500-1000mAh LiPo with TP4056 Charger Module | Or direct USB-C bus power |
| **Enclosure** | 1 | Custom Ergonomic Transmitter Shell | **3D Printed** (PLA/PETG) or layered laser-cut acrylic |

---

## 3. Workshop Consumables & Fasteners

- Ultra-flexible multi-strand silicone hookup wire (28-30 AWG)
- Lead-free solder wire & flux
- Vibration-dampening double-sided foam tape (for IMU decoupling)
- M2 nylon screws, nuts, and standoffs
- Heat-shrink tubing (1mm - 3mm)
