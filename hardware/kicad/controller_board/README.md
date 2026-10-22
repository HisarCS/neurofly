# NeuroFly Handheld Controller PCB (KiCad) 🎮

This directory contains the custom KiCad hardware project files for the **NeuroFly Smart Remote Controller (Transmitter)**, optimized for commercial batch production via **PCBWay** or **JLCPCB**.

---

## 📐 Board Specifications

- **Dimensions:** 85.0 mm × 60.0 mm (handheld gamepad profile with ergonomic corner radii)
- **Mounting Hole Pattern:** 4× M2.5 corner screws for direct fastening into the 3D printed shell (`neurofly_controller_case.stl`)
- **Layer Stackup:** 2-Layer FR4 (1.6mm thickness)
- **Finish:** Matte Black Solder Mask with ENIG (Electroless Nickel Immersion Gold) pads for gold aesthetic

---

## ⚡ Peripheral Connections

1. **MCU Socket:** Female 2.54mm header for Seeed Studio XIAO ESP32-C6.
2. **Display Socket:** 4-pin female header (`GND`, `VCC`, `SCL`, `SDA`) for 0.96" SSD1306 monochrome OLED.
3. **Tactile Switches:** 6× SMD tactile pushbuttons (`6x6mm` or `PTS645`) for Throttle, Pitch, Roll, Arm, and Flight Mode selection.
4. **USB-C Flashing / Simulator Port:** Directly exposed through the XIAO ESP32-C6 USB-C connector for zero-driver Web Serial browser flight simulation.
