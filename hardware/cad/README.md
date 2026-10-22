# NeuroFly 3D CAD & Additive Manufacturing 🖨️

This directory contains the 3D printable meshes (STL) for the **NeuroFly** indoor micro-quadcopter, designed for rapid FDM 3D printing in educational makerspaces.

---

## 📁 Models

1. **`neurofly_prop_duct_40mm.stl`**:
   - Aerodynamic propeller duct guard with integrated motor collar.
   - Encloses 31mm - 40mm micro propellers to guarantee 100% finger and spectator safety during indoor laboratory flight.
   - Print with **TPU 95A** (flexible filament) for virtually unbreakable crash resistance, or **PLA** (0.8mm wall thickness) for ultra-lightweight racing.
2. **`neurofly_battery_tray.stl`**:
   - Modular slide-in cradle for 1S 300mAh - 450mAh LiPo batteries.
   - Secures with M2 nylon screws directly beneath the central chassis frame.
3. **`neurofly_controller_case.stl`**:
   - Ergonomic handheld remote controller enclosure shell.
   - Features dedicated cutouts for a 0.96" I2C OLED display (SSD1306), directional pushbuttons / mini analog thumbsticks, and side USB-C port access.

---

## ⚙️ Slicer Recommendations (PrusaSlicer / Cura / Bambu Studio)

| Parameter | Recommended Setting |
| :--- | :--- |
| **Layer Height** | 0.16 mm - 0.20 mm |
| **Wall Loops / Perimeters** | 2 perimeters (0.8mm total shell) |
| **Infill** | 15% - 20% Gyroid or Grid |
| **Material** | TPU 95A (Preferred) or PLA / PETG |
| **Supports** | None required (designed with 45° self-supporting overhangs) |
| **Part Mass** | Prop duct: ~1.4 grams each / Battery tray: ~0.9 grams |
