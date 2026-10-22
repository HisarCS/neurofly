# The NeuroFly All-in-One Maker Flight Kit 📦🚀
**"Unbox. Solder. Code. Compete."**  
*The Ultimate Hands-On Aerial Robotics Kit for Students*

---

## 🎁 The All-in-One Kit Experience

Before the Grand Prix competition heat, each student team is handed the **NeuroFly Flight Kit Box**—a premium, organized hardware package containing every single raw mechanical, electronic, and semiconductor part needed to construct an indoor micro-quadcopter and dual-mode remote transmitter from the ground up.

Nothing comes pre-assembled or pre-soldered! Students build their own circuit boards, solder real surface-mount semiconductors, test electrical continuity, tune flight firmware, and pilot their own creations in the arena.

---

## 🗃️ Kit Compartment Breakdown (What's in the Box?)

```text
┌────────────────────────────────────────────────────────────────────────┐
│                      THE NEUROFLY MAKER BOX                            │
├──────────────────────────┬─────────────────────────────────────────────┤
│ COMPARTMENT A: MECHANICS │ COMPARTMENT B: CUSTOM CIRCUIT BOARDS        │
│ • Laser-cut birch frame  │ • 1x NeuroFly Drone Carrier PCB (PCBWay)    │
│ • 4x 3D TPU prop ducts   │ • 1x NeuroFly Transmitter PCB (PCBWay)      │
│ • 3D battery cradle      │ • High-spec Matte Black & ENIG Gold finish  │
├──────────────────────────┼─────────────────────────────────────────────┤
│ COMPARTMENT C: AVIONICS  │ COMPARTMENT D: PROPULSION                   │
│ • 2x XIAO ESP32-C6 MCUs  │ • 4x 8520 Coreless DC Motors (2 CW, 2 CCW)  │
│ • 1x MPU6050 6-DOF IMU   │ • 4x 40mm Micro Propellers (2 CW, 2 CCW)    │
│ • 1x 0.96" I2C OLED Disp │ • 4x Anti-vibration silicone motor grommets │
├──────────────────────────┼─────────────────────────────────────────────┤
│ COMPARTMENT E: SMD PARTS │ COMPARTMENT F: POWER & TELEMETRY            │
│ • 4x AO3400A N-MOSFETs   │ • 1x 1S 3.7V 350mAh High-C LiPo Battery     │
│ • 4x SS14 Schottky Diodes│ • 1x JST-PH / BT2.0 Power Pigtail           │
│ • 100Ω & 10kΩ resistors  │ • 1x Braided USB-C Data Cable (Web Serial)  │
│ • 6x Tactile pushbuttons │ • 1x 3D Printed Transmitter Shell           │
└──────────────────────────┴─────────────────────────────────────────────┘
```

---

## 🏭 Industrial PCB Fabrication Notes (PCBWay / JLCPCB)

To give students a commercial, aerospace-grade prototyping experience, the custom carrier boards are manufactured with these exact specifications:

| Parameter | Drone Carrier PCB | Transmitter PCB |
| :--- | :--- | :--- |
| **Dimensions** | 36.0 mm × 36.0 mm | 85.0 mm × 60.0 mm |
| **Layers** | 2-Layer FR4 | 2-Layer FR4 |
| **Board Thickness** | 1.0 mm (Ultra-lightweight) | 1.6 mm (Ergonomic rigidity) |
| **Solder Mask** | Matte Black | Matte Black |
| **Surface Finish** | ENIG (Immersion Gold) | ENIG (Immersion Gold) |
| **Copper Weight** | 1.0 oz (35 &mu;m) | 1.0 oz (35 &mu;m) |

---

## 🧑‍🏫 Teacher Box Preparation Checklist (Per 20 Student Teams)

Lab managers and facilitators can prep 20 student kit boxes in approximately 90 minutes using this workflow:

1. **Laser Cutting Batch:**
   - Place a $600\text{ mm} \times 400\text{ mm}$ 3mm Baltic birch plywood sheet on the laser bed.
   - Run `hardware/laser/neurofly_chassis_frame.dxf` (Array: 4×5 frames). Cut time: ~12 minutes.
2. **3D Printing Batch:**
   - Load Bambu / Prusa printers with flexible TPU 95A for ducts and PLA for battery trays and controller shells.
3. **Component Kitting Trays:**
   - Place semiconductors in anti-static ESD zip pouches (labeled with part numbers to prevent confusion during Week 2 soldering).
4. **Box Sealing:**
   - Affix the official team barcode/number sticker on the box lid.
