# NeuroFly 🚀

**NeuroFly** is an open-source, modular, indoor micro-drone and smart remote controller ecosystem designed for K-12 engineering education, developed within the ethos of **Hisar School IdeaLab / FabLab**, **MIT Edgerton Center Makerspace**, and the **Fab Academy** network.

The platform bridges digital fabrication (laser cutting & 3D printing), embedded systems engineering (Seeed Studio XIAO ESP32-C6), ultra-low-latency wireless telemetry (ESP-NOW), browser-based 3D simulation (Web Serial API), and multi-tier student drone challenges (manual, autonomous, and computer vision / AI).

---

## 🎯 Core Pillars & Vision

1. **Digital Fabrication First:**
   - **Chassis / Airframe:** Parametric laser-cut designs (Acrylic, Baltic Birch Plywood, or Delrin).
   - **Protection & Mounting:** 3D-printed lightweight components (PLA / flexible TPU propeller ducts).
2. **Safe Indoor Architecture:**
   - Powered by 1S LiPo batteries and lightweight coreless DC motors (safer and more accessible than high-KV brushless motors for indoor classrooms).
   - Full prop-guard ducting ensures safe flight in laboratory and classroom environments.
3. **Smart Bi-Directional Transmitter:**
   - Powered by Seeed Studio XIAO ESP32-C6.
   - Onboard I2C OLED display (SSD1306) for real-time telemetry (RSSI, battery, flight modes).
   - Low-latency ESP-NOW RF link for direct drone control.
   - Native USB-C Web Serial integration for instant browser simulator training without installing native drivers.
4. **Interactive 3D Web Simulator:**
   - Built with WebGL / Three.js.
   - Allows students to connect their physical controller and practice flying, test PID tunings, and simulate race gates virtually before maiden flight.
5. **Multi-Disciplinary Competition Matrix:**
   - **Weight Efficiency:** Rewarding the lightest structural designs.
   - **Agility & Endurance:** Lap racing through FabLab gate courses.
   - **Autonomous & AI:** Altitude-hold, ToF laser obstacle avoidance, and visual target tracking.

---

## 📁 Repository Layout

```text
neurofly/
├── firmware/
│   ├── drone/              # Flight controller firmware (PID, IMU, Motor PWM, Failsafe)
│   ├── controller/         # Transmitter firmware (Input parsing, OLED UI, ESP-NOW / Serial)
│   └── common/             # Shared protocol headers and packet definitions (protocol.h)
├── hardware/
│   ├── kicad/              # Custom PCB carrier boards (drone & controller)
│   │   ├── drone_board/    # Flight controller carrier PCB (FETs, IMU, connectors)
│   │   └── controller_board/ # Handheld transmitter PCB
│   ├── cad/                # 3D printable models (STL/STEP: prop ducts, casing, battery trays)
│   ├── laser/              # Laser-cutting vector drawings (DXF/SVG frames)
│   └── schematics/         # Pinout guides, wiring diagrams, and circuit schematics
├── webapp/                 # Browser-based 3D flight simulator & telemetry station
│   └── public/             # Static web app (Three.js + Web Serial API)
└── docs/
    ├── assembly/           # Step-by-step build manuals & student guides
    ├── bom/                # Bill of Materials (BOM) & procurement details
    ├── competition/        # Challenge rules, arena layouts, and scoring formulas
    └── ai_autonomous/      # Autonomous navigation & computer vision specifications
```

---

## 🛠️ Hardware Specifications

- **Main MCU:** [Seeed Studio XIAO ESP32-C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/) (High-performance 32-bit RISC-V, Wi-Fi 6, BLE 5.3, Zigbee/Thread 802.15.4, ESP-NOW)
- **Drone Inertial Unit (IMU):** 6-Axis Gyroscope & Accelerometer (MPU6050 or LSM6DS3 via I2C)
- **Ranging / Obstacle Sensor:** VL53L0X / VL53L1X Time-of-Flight (ToF) distance sensor
- **Motors:** 4x Coreless Brushed DC Motors (7x16mm 716 or 8.5x20mm 8520)
- **Motor Drivers:** 4x Logic-Level N-Channel MOSFETs (AO3400 / SI2302) with Flyback Schottky Diodes (1N5819 / SS14)
- **Propellers:** 31mm - 55mm Micro Propellers (2x CW, 2x CCW)
- **Controller Interface:** 0.96" I2C OLED (SSD1306, 128x64) + Directional Push Buttons / Mini Analog Thumbsticks
- **Power:** 1S 3.7V 300mAh - 450mAh LiPo Battery (25C-30C discharge)

---

## 🚀 Quick Start: Web Simulator

Test the flight dynamics and connect your controller right in the browser:

```bash
cd webapp/public
python3 -m http.server 8000
```
Open `http://localhost:8000` in Google Chrome or Microsoft Edge. Plug in your XIAO ESP32-C6 controller via USB-C and click **"Connect Controller"**.

---

## 📜 License
Developed for educational and FabLab community advancement. Open source under the MIT License.
