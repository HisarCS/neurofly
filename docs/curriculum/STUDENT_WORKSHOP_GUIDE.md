# NeuroFly Student Workshop Curriculum 🎓
**A 4-Week Hands-On Aerial Robotics & Digital Fabrication Journey**  
*Hisar IdeaLab / FabLab Network / MIT Edgerton Center Makerspace*

---

## 🗓️ Workshop Overview

NeuroFly teaches students how to design, fabricate, assemble, program, and pilot an indoor micro-quadcopter and custom remote transmitter.

| Week | Focus Area | Core Skills & Deliverables |
| :---: | :--- | :--- |
| **Week 1** | **Digital Fabrication & Airframe CAD** | Laser cutting chassis, 3D printing TPU prop guards, mass optimization |
| **Week 2** | **Electronics Production & Assembly** | KiCad PCB architecture, SMD soldering, circuit inspection & safety |
| **Week 3** | **Firmware, Telemetry & Web Simulator** | ESP32-C6 setup, ESP-NOW protocol, flight training in 3D simulator |
| **Week 4** | **Flight Tuning & Grand Prix Arena** | PID stabilization, gate course racing, autonomous challenge missions |

---

## 🚀 Week 1: Digital Fabrication & Airframe CAD

### Objectives
- Understand quadcopter aerodynamics: thrust, weight, drag, and moment of inertia.
- Master laser cutting kerf compensation and 3D print slicing for ultra-lightweight structures.

### Hands-on Activities
1. **Chassis Laser Cutting:**
   - Import `hardware/laser/frame.dxf` into the FabLab laser software (LightBurn / LaserCut).
   - Cut test frames using 2.0mm acrylic or 3.0mm Baltic birch plywood.
   - Measure frame weight with precision scale (Goal: Frame < 6.0 grams).
2. **Prop Guard 3D Printing:**
   - Slice the cylindrical ducts using Cura or PrusaSlicer.
   - Print with 95A TPU (flexible) or ultra-thin PLA (0.8mm wall thickness, single perimeter).
   - Test fit 716 / 8520 coreless motors into the motor sleeves.

---

## ⚡ Week 2: Electronics Production & Assembly

### Objectives
- Understand N-channel MOSFET switching, flyback inductive spikes, and LiPo safety.
- Acquire precision hand-soldering skills for 0805 SMD components and headers.

### Hands-on Activities
1. **PCB Inspection & Milling:**
   - Review schematic nets in KiCad (`hardware/kicad/drone_board/`).
   - Mill the single-sided PCB on the Roland SRM-20 CNC or inspect fabricated boards.
2. **Soldering Walkthrough:**
   - Solder gate resistors (100Ω) and pull-down resistors (10kΩ).
   - Align and solder the AO3400 MOSFETs (SOT-23) and 1N5819 Schottky diodes.
   - Install XIAO ESP32-C6 pin headers and MPU6050 IMU mount.
3. **Safety Inspection:**
   - Multimeter continuity check between VCC and GND (no shorts allowed!).
   - Power up with current-limited power supply (3.7V, 0.2A limit) before attaching LiPo.

---

## 💻 Week 3: Firmware, Telemetry & Web Simulation

### Objectives
- Explore embedded microcontroller programming on 32-bit RISC-V (ESP32-C6).
- Learn how wireless ESP-NOW packets transfer control inputs with <5ms latency.
- Practice flight dynamics in the zero-risk 3D browser simulator.

### Hands-on Activities
1. **Transmitter Assembly & Firmware:**
   - Connect OLED display (I2C) and navigation push buttons to the transmitter XIAO.
   - Flash `firmware/controller/main.cpp` using Arduino IDE / PlatformIO.
2. **Hardware-in-the-Loop Web Simulation:**
   - Connect the transmitter to a PC via USB-C.
   - Launch `http://localhost:8000` (or `https://HisarCS.github.io/neurofly`).
   - Click **"Connect Web Serial Controller"** and navigate through virtual gates.
   - Analyze how roll, pitch, yaw, and throttle inputs affect quadcopter trajectory.

---

## 🏆 Week 4: Flight Tuning & Challenge Day

### Objectives
- Calibrate the MPU6050 6-DOF IMU and tune motor mixer balances.
- Compete in the three-pillar Grand Prix: Weight Efficiency, Lap Agility, and Autonomous missions.

### Hands-on Activities
1. **Bench Motor Testing:**
   - Secure the quadcopter to the test bench.
   - Test motor rotation directions:
     - Front-Left & Rear-Right: CW (Clockwise)
     - Front-Right & Rear-Left: CCW (Counter-Clockwise)
2. **Technical Inspection (Scrutineering):**
   - Official weigh-in (Goal: < 45.0g AUW).
   - Failsafe test (turn off transmitter while armed; motors must cut in <200ms).
3. **Grand Prix Arena Competition:**
   - 3-Minute Lap Race through FabLab gates.
   - Autonomous altitude-hold and ToF obstacle avoidance challenge.
