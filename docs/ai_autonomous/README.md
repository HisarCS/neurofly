# NeuroFly AI & Autonomous Vision Track 👁️🤖

This module provides the computer vision pipeline for the **AI Challenge Track** of the NeuroFly Grand Prix (+100 Points); a competition for high-school and first-year university students is planned for the future. University-extension ideas: camera calibration, `solvePnP` pose estimation, and a filtered position estimate.

---

## 🎯 Challenge Objective

The quadcopter or an overhead FabLab camera station must localize an **ArUco visual marker** (`DICT_4X4_50`, Marker ID: `0`) on the competition landing pad, calculate positional error vectors in real-time, and execute an autonomous pinpoint descent without pilot stick intervention.

---

## 💻 Quick Start

1. Install Python dependencies:
   ```bash
   pip install opencv-python opencv-contrib-python numpy
   pip install pyserial        # only if you want to feed the controller
   ```
2. Connect your USB camera / webcam pointing downward over the landing area.
3. Run the tracker (display only):
   ```bash
   python3 aruco_tracker.py                 # --camera N picks another camera
   ```
4. Place the target ArUco marker in the camera view. The green line is the error vector.
   The tracker computes corrective Roll & Pitch (protocol units, clipped to +/-300) using a
   low-pass filter, a pixel deadband and a small D term so it does not jitter.

## 🔌 Feeding the controller (AI_ASSIST mode)

Plug the NeuroFly **controller** into the PC over USB-C and pass its serial port:

```bash
python3 aruco_tracker.py --port /dev/ttyACM0      # Windows: --port COM5
```

The tracker sends one JSON line at ~20 Hz: `{"vr":<roll>,"vp":<pitch>}`. When the marker is lost
(and on exit) it sends zeros. The controller applies these only while in `AI_ASSIST` mode; the
pilot keeps throttle and the emergency cut. The same USB port carries the controller's outbound
telemetry JSON (`armed, thr, r, p, y, mode, bat, rssi, ver`); the tracker ignores it.

> Status: the serial output is implemented but has not been tested against real controller
> hardware. Bench-test with props OFF first. Camera axes vs. drone axes (sign of roll/pitch)
> depend on how the camera is mounted; flip the sign in `aruco_tracker.py` if the drone
> moves away from the marker instead of toward it.
