# NeuroFly Laser Cutting Specifications ⚡

This folder contains the parametric 2D vector files to manufacture the **NeuroFly Quadcopter Airframe** using FabLab laser cutting systems (Epilog, Trotec, Universal, or Chinese CO2 laser cutters).

---

## 📁 Files

- **`neurofly_chassis_frame.dxf`**: Industry standard AutoCAD R12 DXF format with vector cutting geometry on layer `CUT`.
- **`neurofly_chassis_frame.svg`**: Dimensioned vector graphic (120mm × 120mm canvas) with red hairline vector cut paths (`#FF0000`, 0.1px).

---

## 📐 Dimensional Geometry

- **Diagonal Wheelbase:** 85.0 mm motor-to-motor.
- **Flight Controller Bay:** 36.0 mm × 36.0 mm with 30.5 mm × 30.5 mm standard M2 mounting holes.
- **Motor Pod Sizing:** 8.5 mm diameter inner hole designed for friction fit of 8520 coreless motors (or 7.0 mm adapter for 716 motors).
- **Weight Reduction:** 14.0 mm × 14.0 mm center cutout and battery tie slots.

---

## 🛠️ FabLab CO2 Laser Parameters (Reference for 50W - 80W)

| Material | Thickness | Speed | Power | Frequency | Notes |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **Baltic Birch Plywood** | 2.5 - 3.0 mm | 18 - 22 mm/s | 65 - 75% | 500 Hz | Best strength-to-weight ratio; rigid flight characteristics. |
| **Cast Acrylic (PMMA)** | 2.0 - 3.0 mm | 12 - 15 mm/s | 70 - 80% | 5000 Hz | Crystal clear or neon aesthetic; flame polished edge. |
| **Delrin (Acetal)** | 2.0 mm | 15 - 20 mm/s | 60 - 70% | 1000 Hz | Impact-resistant; excellent crash resilience. |

> [!TIP]
> Keep the beam focal length set precisely with the focus gauge. A 0.15mm beam kerf ensures a snug friction fit for 8520 motor cans without needing glue.
