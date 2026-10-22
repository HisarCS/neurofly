"""
NeuroFly Overhead Vision & ArUco Autonomous Landing Tracker
============================================================
Detects the ArUco landing-pad marker with an overhead camera and computes
corrective Roll / Pitch commands. Optionally streams them to the NeuroFly
controller over USB serial (controller flight mode AI_ASSIST) as one JSON
line per update:

    {"vr": <roll correction>, "vp": <pitch correction>}

Values are in protocol units (-500..+500, clipped here to +/-300). When the
target is lost the tracker sends zeros so the drone is never fed stale data.

Usage:
  python3 aruco_tracker.py                      # display only
  python3 aruco_tracker.py --camera 1           # pick another camera
  python3 aruco_tracker.py --port /dev/ttyACM0  # also send to controller
  python3 aruco_tracker.py --port COM5 --baud 115200

Requirements:
  pip install opencv-python opencv-contrib-python numpy
  pip install pyserial          # only needed with --port
"""

import argparse
import json
import sys
import time

import cv2
import cv2.aruco as aruco
import numpy as np

# Marker Configuration
ARUCO_DICT = aruco.getPredefinedDictionary(aruco.DICT_4X4_50)
TARGET_MARKER_ID = 0   # Default NeuroFly Landing Pad Marker ID
MARKER_SIZE_MM = 80.0  # Physical marker edge length in mm

# Control tuning (pixels -> protocol units)
KP = 1.5               # proportional gain
KD = 0.6               # derivative gain (applied to low-passed error, per second scale below)
LPF_ALPHA = 0.35       # low-pass factor for pixel error (0..1, smaller = smoother)
DEADBAND_PX = 8        # ignore errors smaller than this (prevents jitter at the centre)
CORR_LIMIT = 300       # max |correction| sent
SEND_HZ = 20.0         # serial update rate


def compute_correction(err, prev_err, dt):
    """P + D on a low-passed pixel error with a deadband. Returns (corr, new_err)."""
    if abs(err) < DEADBAND_PX:
        err = 0.0
    d = (err - prev_err) / dt if dt > 0 else 0.0
    corr = KP * err + KD * d * 0.05  # 0.05 s scale keeps D comparable to P in px units
    return float(np.clip(corr, -CORR_LIMIT, CORR_LIMIT))


def open_serial(port, baud):
    try:
        import serial  # pyserial
    except ImportError:
        print("[Error] --port needs pyserial: pip install pyserial")
        sys.exit(1)
    try:
        return serial.Serial(port, baud, timeout=0, write_timeout=0.05)
    except Exception as exc:
        print(f"[Error] Could not open serial port {port}: {exc}")
        sys.exit(1)


def send_corrections(ser, roll, pitch):
    if ser is None:
        return
    line = json.dumps({"vr": int(roll), "vp": int(pitch)}, separators=(",", ":")) + "\n"
    try:
        ser.write(line.encode("ascii"))
    except Exception as exc:  # write timeout / unplugged: keep the display running
        print(f"[Warn] Serial write failed: {exc}")


def main():
    parser = argparse.ArgumentParser(description="NeuroFly ArUco overhead tracker")
    parser.add_argument("--camera", type=int, default=0, help="camera index (default 0)")
    parser.add_argument("--port", default=None,
                        help="serial port of the NeuroFly controller (default: off)")
    parser.add_argument("--baud", type=int, default=115200, help="serial baud (default 115200)")
    args = parser.parse_args()

    print("[NeuroFly Vision] Initializing Overhead Camera Station...")
    cap = cv2.VideoCapture(args.camera)

    if not cap.isOpened():
        print(f"[Error] Could not open video device {args.camera}.")
        return

    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

    ser = open_serial(args.port, args.baud) if args.port else None
    if ser:
        print(f"[NeuroFly Vision] Sending corrections to {args.port} @ {args.baud} at {SEND_HZ:.0f} Hz")

    parameters = aruco.DetectorParameters()
    detector = aruco.ArucoDetector(ARUCO_DICT, parameters)

    print("[NeuroFly Vision] Tracking Active. Press 'q' to exit.")

    # Filter state
    fx = fy = 0.0            # low-passed errors
    prev_fx = prev_fy = 0.0
    locked_before = False
    corr_roll = corr_pitch = 0
    last_send = 0.0
    last_t = time.monotonic()

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            now = time.monotonic()
            dt = max(now - last_t, 1e-3)
            last_t = now

            h, w, _ = frame.shape
            center_x, center_y = w // 2, h // 2

            # Draw Arena Center Crosshairs
            cv2.line(frame, (center_x - 20, center_y), (center_x + 20, center_y), (0, 255, 255), 2)
            cv2.line(frame, (center_x, center_y - 20), (center_x, center_y + 20), (0, 255, 255), 2)
            cv2.circle(frame, (center_x, center_y), 40, (0, 255, 255), 1)

            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            corners, ids, rejected = detector.detectMarkers(gray)

            if ids is not None and TARGET_MARKER_ID in ids:
                aruco.drawDetectedMarkers(frame, corners, ids)
                idx = np.where(ids == TARGET_MARKER_ID)[0][0]
                pts = corners[idx][0]

                marker_cx = int(np.mean(pts[:, 0]))
                marker_cy = int(np.mean(pts[:, 1]))

                error_x = marker_cx - center_x  # Positive: marker is to the right
                error_y = center_y - marker_cy  # Positive: marker is forward

                if not locked_before:           # re-seed filter on (re)acquire
                    fx, fy = float(error_x), float(error_y)
                    prev_fx, prev_fy = fx, fy
                    locked_before = True
                fx += LPF_ALPHA * (error_x - fx)
                fy += LPF_ALPHA * (error_y - fy)

                corr_roll = int(compute_correction(-fx, -prev_fx, dt))
                corr_pitch = int(compute_correction(fy, prev_fy, dt))
                prev_fx, prev_fy = fx, fy

                cv2.line(frame, (center_x, center_y), (marker_cx, marker_cy), (0, 255, 0), 2)
                cv2.circle(frame, (marker_cx, marker_cy), 5, (0, 0, 255), -1)

                status_text = (f"DX: {error_x:+d}px | DY: {error_y:+d}px -> "
                               f"Roll: {corr_roll:+d} Pitch: {corr_pitch:+d}")
                cv2.putText(frame, status_text, (20, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
                cv2.putText(frame, "TARGET LOCKED", (20, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
            else:
                # Target lost: zero the commands and reset the filters.
                locked_before = False
                corr_roll = corr_pitch = 0
                fx = fy = prev_fx = prev_fy = 0.0
                cv2.putText(frame, "SEARCHING FOR TARGET...", (20, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)

            if ser and (now - last_send) >= 1.0 / SEND_HZ:
                send_corrections(ser, corr_roll, corr_pitch)
                last_send = now

            cv2.imshow("NeuroFly AI Vision Tracker", frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    finally:
        send_corrections(ser, 0, 0)  # never leave a stale correction behind
        if ser:
            ser.close()
        cap.release()
        cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
