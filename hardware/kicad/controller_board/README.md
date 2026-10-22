# NeuroFly Handheld Controller PCB (KiCad) 🎮

This directory contains the custom KiCad hardware project files for the **NeuroFly Smart Remote Controller (Transmitter)**, optimized for commercial batch production via **PCBWay** or **JLCPCB**.

---

## 📐 Board Specifications

- **Dimensions:** 85.0 mm × 60.0 mm (handheld gamepad profile with ergonomic corner radii)
- **Mounting Hole Pattern:** 4× M2.5 corner screws for direct fastening into the 3D printed shell (`neurofly_controller_case.stl`)
- **Layer Stackup:** 2-Layer FR4 (1.6mm thickness)
- **Finish:** Matte Black Solder Mask with ENIG (Electroless Nickel Immersion Gold) pads for gold aesthetic

---

## ⚠️ Project status: SKELETON, NOT A MANUFACTURABLE DESIGN

Honest inspection of the files in this directory (v0.2.0):

- `neurofly_controller.kicad_sch`: contains the title block and a **library of symbol definitions only**. There are **no placed component instances** (no `(symbol (lib_id ...))` entries), no net labels, and a single stray wire. It is not a complete schematic, ERC has not been run, and nothing is wired.
- `neurofly_controller.kicad_pcb`: contains only an 85 x 60 mm board outline and silkscreen text. It has **no footprints, tracks, vias, zones or mounting holes**.
- **No DRC has been run and the boards are NOT routed. Do not send these files to a fab.** Treat them as a starting point: the board outline, mounting holes and silkscreen labels are a head start, the circuit still has to be drawn and routed.
- The XIAO symbol pin names for D8/D9/D10 were corrected in v0.2.0 to GPIO19/GPIO20/GPIO18 (see `hardware/schematics/PINOUT.md`; verify against the Seeed pinout before ordering).
- Until the KiCad design is completed, build the electronics from `hardware/schematics/PINOUT.md` on perfboard or a milled board you design yourself, or complete the KiCad project as a student/teacher exercise.

---

## ⚡ Peripheral Connections (design intent)

1. **MCU Socket:** Female 2.54mm header for Seeed Studio XIAO ESP32-C6.
2. **Display Socket:** 4-pin female header (`GND`, `VCC`, `SCL`, `SDA`) for 0.96" SSD1306 monochrome OLED.
3. **Inputs (choose one variant, see `hardware/schematics/PINOUT.md`):**
   - *Analog gimbals:* two 2-axis thumbsticks on D0 throttle, D1 yaw, D2 pitch, D3 roll, powered from **3V3** (not 5V). The ESP32-C6 ADC exists only on GPIO0-6, so this variant uses an external **ADS1115** (I2C 0x48 on D4/D5, shared with the OLED): A0 throttle, A1 yaw, A2 pitch, A3 roll; D0-D3 stay unused. Firmware: `-DINPUT_MODE=1`.
   - *Buttons (firmware default):* 4 tactile switches on D0-D3 (MODE held = shift for yaw/roll), switches to GND using the internal pull-ups (firmware `INPUT_PULLUP`).
   - In both variants: Arm on D8, Mode on D9 (tactile switch or toggle to GND); OLED on D4/D5 (I2C 0x3C).
4. **USB-C Flashing / Simulator Port:** Directly exposed through the XIAO ESP32-C6 USB-C connector for zero-driver Web Serial browser flight simulation.
