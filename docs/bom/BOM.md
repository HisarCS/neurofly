# NeuroFly - Bill of Materials (BOM)

This document provides a comprehensive component breakdown for the **NeuroFly Drone** and the **NeuroFly Smart Remote Controller**, including digital fabrication materials compatible with standard FabLab / makerspace equipment.

---

## 1. NeuroFly Drone (Flight Platform)

| Component | Qty | Part / Specifications | FabLab Notes |
| :--- | :---: | :--- | :--- |
| **Microcontroller** | 1 | Seeed Studio XIAO ESP32-C6 | Onboard ceramic antenna or external U.FL |
| **IMU Sensor** | 1 | MPU6050 or LSM6DS3 breakout | 6-DOF Gyroscope + Accelerometer (I2C) |
| **Ranging Sensor (Optional)** | 1 | VL53L0X or VL53L1X | ToF Laser Distance Sensor (Altitude Hold & Obstacle Avoidance) |
| **Motors** | 4 | 716 (7x16mm) or 8520 Coreless DC | 2x CW (Clockwise), 2x CCW (Counter-Clockwise) |
| **Propellers** | 4 | 31mm - 55mm Micro Propellers | 2x CW, 2x CCW |
| **Motor Drivers** | 4 | Logic-Level N-MOSFET (AO3400 / SI2302) | SOT-23 packages with 3.3V logic gate saturation |
| **Flyback Diodes** | 4 | Schottky Diode (1N5819 or SS14) | Absorbs motor inductive back-EMF spikes |
| **Passive Resistors** | 4 | 10kΩ (Gate Pull-Down) & 4x 100Ω (Gate Limit) | 0805 or 1206 SMD / Through-hole |
| **Battery** | 1 | 1S 3.7V 300mAh - 450mAh LiPo (25C-30C) | JST-PH 2.0 or BT2.0 connector |
| **Main Chassis** | 1 | 2mm / 3mm Cast Acrylic or Baltic Birch | **Laser Cut** (FabLab CO2 Laser) |
| **Prop Guards & Ducts** | 1-4 | Lightweight TPU / PLA | **3D Printed** (FDM, 0.2mm layer height) |

---

## 2. NeuroFly Remote Controller (Transmitter)

| Component | Qty | Part / Specifications | FabLab Notes |
| :--- | :---: | :--- | :--- |
| **Microcontroller** | 1 | Seeed Studio XIAO ESP32-C6 | Dual-mode: ESP-NOW RF link & USB-C Web Serial |
| **Display** | 1 | 0.96" I2C Monochrome OLED (SSD1306, 128x64) | Shows battery, RSSI, telemetry, mode |
| **Inputs** | 4-6 | 12x12mm Tactile Buttons or 2x Mini Analog Joysticks | Throttle, Yaw, Pitch, Roll, Arm, Mode switch |
| **Battery & Charging** | 1 | 1S 500-1000mAh LiPo with TP4056 Charger Module | Or direct USB-C bus power |
| **Enclosure** | 1 | Custom Ergonomic Transmitter Shell | **3D Printed** (PLA/PETG) or layered laser-cut acrylic |

---

## 3. Workshop Consumables & Fasteners

- Ultra-flexible multi-strand silicone hookup wire (28-30 AWG)
- Lead-free solder wire & flux
- Vibration-dampening double-sided foam tape (for IMU decoupling)
- M2 nylon screws, nuts, and standoffs
- Heat-shrink tubing (1mm - 3mm)
