# NeuroFly Drone Carrier PCB (KiCad) 🛠️

This directory contains the custom KiCad hardware project files for the **NeuroFly Quadcopter Flight Controller Carrier Board**, powered by the **Seeed Studio XIAO ESP32-C6**.

---

## 📐 Board Specifications

- **Dimensions:** 36.0 mm × 36.0 mm (standard micro quadcopter form factor)
- **Mounting Hole Pattern:** 30.5 mm × 30.5 mm (M2 nylon hardware)
- **Center Cutout:** 12.0 mm × 12.0 mm internal cutout to reduce All-Up-Weight (AUW)
- **Substrate:** 1.0mm or 1.2mm FR4 (recommended for low weight) / Single-sided copper for FabLab CNC milling
- **Target Weight:** ~2.8 grams (bare PCB)

---

## ⚡ Circuit Architecture

1. **Microcontroller Socket:**
   - 2× 7-pin 2.54mm pitch headers or direct surface-mount soldering for Seeed Studio XIAO ESP32-C6.
2. **Motor Driver Subsystem (4 Channels):**
   - **MOSFETs:** AO3400A (30V, 5.7A N-Channel MOSFET in SOT-23)
   - **Flyback Protection:** 1N5819 / SS14 Schottky barrier diodes across motor terminals
   - **Gate Drive:** 100Ω series resistor + 10kΩ pull-down to ensure zero throttle upon boot
3. **Sensor Bus (I2C):**
   - 4-Pin header (`3V3`, `GND`, `SDA/GPIO22`, `SCL/GPIO23`) for MPU6050 6-DOF IMU and optional VL53L0X ToF laser ranger.
4. **Telemetry & Safety:**
   - Voltage divider (2× 10kΩ 1% resistors) mapping 1S LiPo (3.0V - 4.2V) to 1.5V - 2.1V for ADC readout on XIAO pin `D6/GPIO16`.
   - Onboard Status LED on `D7/GPIO17`.

---

## 🏭 Digital Fabrication Guide (FabLab Roland SRM-20)

Students can mill this PCB in the FabLab using standard PCB milling machines:

1. **Isolation Milling:**
   - Endmill: 1/64" flat endmill (or 0.2mm - 0.4mm V-bit 30°)
   - Cut depth: 0.04 mm - 0.08 mm
   - Spindle speed: 8,000 - 15,000 RPM
   - Feedrate: 3 - 4 mm/s
2. **Drilling & Edge Cutout:**
   - Endmill: 1/32" flat endmill
   - Multi-pass cutting: 0.4mm depth per pass through 1.2mm FR1 / copper clad board.

---

## 🧑‍🎓 Student Soldering Sequence (Step-by-Step)

1. **Passives First (SMD 0805):** Solder 100Ω and 10kΩ resistors.
2. **Semiconductors:** Solder the four AO3400 SOT-23 MOSFETs and SS14 Schottky diodes (note polarity line towards +3.7V).
3. **Decoupling Capacitor:** Solder 10uF ceramic / low-ESR capacitor near the battery pad.
4. **Headers / Module:** Solder female headers for XIAO ESP32-C6 and the I2C IMU board.
5. **Power Cable:** Solder JST-PH 2.0 or BT2.0 pigtail with correct polarity (Red = +3.7V, Black = GND).
6. **Continuity & Short-Circuit Check:** Verify with multimeter in continuity mode that +3.7V and GND are NOT shorted before connecting battery!
