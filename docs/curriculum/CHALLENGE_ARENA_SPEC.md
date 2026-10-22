# NeuroFly Arena Specifications & Course Setup 🏟️

This document outlines the physical dimensions, materials, and safety infrastructure required to construct the **NeuroFly Indoor Challenge Arena** for a classroom challenge, or a future competition, in a standard school FabLab, gymnasium, or makerspace.

---

## 📐 Arena Geometry

- **Recommended Floor Area:** 6.0 meters × 8.0 meters (minimum 4.0m × 5.0m)
- **Clear Ceiling Height:** Minimum 2.5 meters
- **Safety Containment Netting:** 
  - Ultra-fine mesh nylon netting (mesh size ≤ 15mm) along the perimeter to safeguard spectators and computer workstations.
  - Suspended with carabiners from ceiling hooks or freestanding aluminum truss / PVC uprights.

---

## 🚪 FabLab Race Gates (DIY Digital Fabrication)

The course features **4 illuminated gates** in a clockwise slalom loop:

1. **Materials:**
   - Standard 20mm (3/4") white PVC plumbing pipe & 90° elbow connectors.
   - Laser-cut 3mm neon acrylic arch plates engraved with gate numbers (Gate 1, 2, 3, Finish).
   - WS2812B addressable LED strips wrapped around the gate perimeter (5V powered via USB power bank).
2. **Gate Dimensions:**
   - Inner clearance opening: 80 cm wide × 80 cm high.
   - Elevation above floor:
     - Gate 1: 30 cm ground clearance (low gate)
     - Gate 2: 70 cm ground clearance (elevated hurdle)
     - Gate 3: 40 cm ground clearance (slalom turn)
     - Gate 4 (Finish/Start): 50 cm ground clearance

---

## 🎯 Autonomous & AI Challenge Stations

1. **Precision Landing Pad:**
   - 40cm × 40cm laser-engraved pad with concentric high-contrast rings and center target.
   - Central ArUco Marker (Dictionary: `DICT_4X4_50`, Marker ID: `0`) for OpenCV / camera-assisted autonomous touchdown.
2. **Obstacle Alley (ToF Challenge):**
   - 2.0m wide corridor with foam pylons (lightweight foam noodles).
   - Drone must cruise autonomously without exceeding 50cm altitude and avoid contact with obstacles.
