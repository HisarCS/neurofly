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

## ⚠️ Project status: SKELETON, NOT A MANUFACTURABLE DESIGN

Honest inspection of the files in this directory (v0.2.0):

- `neurofly_drone.kicad_sch`: contains the title block and a **library of symbol definitions only**. There are **no placed component instances** (no `(symbol (lib_id ...))` entries), no net labels, and a single stray wire. It is not a complete schematic, ERC has not been run, and nothing is wired.
- `neurofly_drone.kicad_pcb`: contains the 36 x 36 mm outline, a 12 x 12 mm centre cutout, and silkscreen text (title, `M1-FL`..`M4-RL`). It has **no footprints, no tracks, no vias, no copper zones** and no mounting-hole footprints (the 30.5 mm hole pattern is described in text only).
- **No DRC has been run and the boards are NOT routed. Do not send these files to a fab.** Treat them as a starting point: the board outline, mounting holes and silkscreen labels are a head start, the circuit still has to be drawn and routed.
- The XIAO symbol pin names for D8/D9/D10 were corrected in v0.2.0 to GPIO19/GPIO20/GPIO18 (see `hardware/schematics/PINOUT.md`; verify against the Seeed pinout before ordering).
- Until the KiCad design is completed, build the electronics from `hardware/schematics/PINOUT.md` on perfboard or a milled board you design yourself, or complete the KiCad project as a student/teacher exercise.

---

## ⚡ Circuit Architecture (design intent)

1. **Microcontroller Socket:**
   - 2× 7-pin 2.54mm pitch headers or direct surface-mount soldering for Seeed Studio XIAO ESP32-C6.
2. **Motor Driver Subsystem (4 Channels):**
   - **MOSFETs:** AO3400A (30V, 5.7A N-Channel MOSFET in SOT-23)
   - **Flyback Protection:** SS14 (SMA footprint) Schottky barrier diodes across motor terminals (1N5819 only as a through-hole DO-41 alternative)
   - **Gate Drive:** 100Ω series resistor + 10kΩ pull-down per channel (4+4 resistors) to ensure zero throttle upon boot; AO3400A is fully enhanced at 3.3 V gate drive
3. **Sensor Bus (I2C):**
   - 4-Pin header (`3V3`, `GND`, `SDA/GPIO22`, `SCL/GPIO23`) for MPU6050 6-DOF IMU and optional VL53L0X or VL53L1X ToF laser ranger (same I2C address 0x29; firmware 0.3.0 auto-detects).
4. **Telemetry & Safety:**
   - Voltage divider (2× 10kΩ 1% resistors) mapping 1S LiPo (3.0V - 4.2V) to 1.5V - 2.1V for ADC readout. Battery sense goes to **`D2/GPIO2` (ADC1_CH2)**; the Rear-Right motor gate net is on **`D6/GPIO16`** (motors: D0 FL, D1 FR, D6 RR, D3 RL). See `hardware/schematics/PINOUT.md`. Note: the symbol library in the .kicad_sch is unchanged and no nets are placed yet.
   - Status LED / buzzer on `D7/GPIO17`; optional ToF XSHUT on `D8/GPIO19`.
   - Flyback diode: **use SS14 in the SMA footprint (`D_SMA`)**. The symbol in the .kicad_sch currently carries `D_SOD-123`, which does not match SS14 (SMA) or 1N5819 (DO-41); change the footprint to `D_SMA` when parts are placed. Known mismatch, not yet fixed in the .kicad files.

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

1. **Passives First (SMD 0805):** Solder 4× 100Ω gate resistors, 4× 10kΩ gate pull-downs and 2× 10kΩ battery-divider resistors.
2. **Semiconductors:** Solder the four AO3400 SOT-23 MOSFETs and SS14 Schottky diodes (note polarity line towards +3.7V).
3. **Decoupling Capacitor:** Solder 10uF ceramic / low-ESR capacitor near the battery pad.
4. **Headers / Module:** Solder female headers for XIAO ESP32-C6 and the I2C IMU board.
5. **Power Cable:** Solder JST-PH 2.0 or BT2.0 pigtail with correct polarity (Red = +3.7V, Black = GND).
6. **Continuity & Short-Circuit Check:** Verify with multimeter in continuity mode that +3.7V and GND are NOT shorted before connecting battery!
