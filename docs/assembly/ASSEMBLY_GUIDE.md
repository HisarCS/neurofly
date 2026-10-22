# NeuroFly Step-by-Step Assembly Manual 🛠️
*A Visual Hardware Assembly Guide for Students & Makers*

---

## 🧰 Required Tools & Consumables
- M2 hex screwdriver / tweezers
- Temperature-controlled soldering iron ($330^\circ\text{C}$) & lead-free solder
- Multimeter with continuity beep mode
- 4× M2 × 8mm nylon screws, nuts, and 4mm standoffs
- High-adhesion foam tape (for IMU vibration isolation)
- 2× Small zip-ties (100mm × 2.5mm)

---

## 🧩 Step 1: Airframe & Prop Guard Integration

```text
       [Prop Duct FL]            [Prop Duct FR]
              \                        /
               \                      /
                ┌────────────────────┐
                │  Laser-Cut Chassis │
                │  (85mm Wheelbase)  │
                └────────────────────┘
               /                      \
              /                        \
       [Prop Duct RL]            [Prop Duct RR]
```

1. Take the laser-cut Baltic birch or acrylic spine (`neurofly_chassis_frame.dxf`).
2. Slide the four 3D-printed TPU prop ducts (`neurofly_prop_duct_40mm.stl`) over the chassis arm ends.
3. Press the 8520 coreless motors gently into the motor collars:
   - **Front-Left & Rear-Right:** Install **CW motors** (Red [+] / Blue [-] wires).
   - **Front-Right & Rear-Left:** Install **CCW motors** (Black [-] / White [+] wires).
4. Route motor wires through the chassis arm wire guide slots towards the center.

---

## ⚡ Step 2: PCB Mounting & Wiring

1. Mount the NeuroFly Flight Controller Carrier PCB onto the central 30.5mm × 30.5mm holes using M2 nylon standoffs.
2. Trim motor wires to length and solder them to the corresponding corner solder pads:
   - `M1_FL` (Front-Left)
   - `M2_FR` (Front-Right)
   - `M3_RR` (Rear-Right)
   - `M4_RL` (Rear-Left)
3. Solder the JST-PH 2.0 or BT2.0 battery lead to the center battery pads. **Double-check polarity! Red is VCC (+3.7V), Black is GND.**

---

## 🛡️ Step 3: Sensor Decoupling & Battery Tray

1. Apply a 10mm × 10mm square of vibration-dampening foam tape beneath the MPU6050 IMU breakout.
2. Press the IMU gently onto the carrier board header. Ensure the IMU X-axis arrow points toward the front of the drone.
3. Attach the 3D-printed battery cradle (`neurofly_battery_tray.stl`) to the bottom of the chassis using two M2 nylon screws.
4. Slide your 1S LiPo (300mAh - 450mAh) into the cradle and secure it with a small zip-tie.

---

## 🔌 Step 4: Pre-Flight Safety Verification

Before plugging in the battery:
- [ ] Multimeter test between LiPo (+) and (-) shows **NO short circuit**.
- [ ] Propellers are **NOT** installed yet.
- [ ] Connect USB-C to the XIAO ESP32-C6 and verify the power LED illuminates solid green.
- [ ] Flash `firmware/drone/main.cpp`.
- [ ] Disconnect USB-C, insert the 1S battery, and verify status LED blinks ready.
- [ ] Now, push the propellers onto the motor shafts:
  - Orange / White props on Front motors (orientation awareness).
  - Black props on Rear motors.
  - Verify CW props on CW motors, CCW props on CCW motors!
