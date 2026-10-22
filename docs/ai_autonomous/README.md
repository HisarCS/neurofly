# NeuroFly AI & Autonomous Vision Track 👁️🤖

This module provides the computer vision pipeline for the **AI Challenge Track** of the NeuroFly Grand Prix (+100 Points).

---

## 🎯 Challenge Objective

The quadcopter or an overhead FabLab camera station must localize an **ArUco visual marker** (`DICT_4X4_50`, Marker ID: `0`) on the competition landing pad, calculate positional error vectors in real-time, and execute an autonomous pinpoint descent without pilot stick intervention.

---

## 💻 Quick Start

1. Install Python dependencies:
   ```bash
   pip install opencv-python opencv-contrib-python numpy
   ```
2. Connect your USB camera / webcam pointing downward over the landing area.
3. Run the tracker:
   ```bash
   python3 aruco_tracker.py
   ```
4. Place the target ArUco marker in the camera view:
   - Green crosshair indicates error vector.
   - Outputs proportional corrective Roll & Pitch values formatted to match `protocol.h`.
