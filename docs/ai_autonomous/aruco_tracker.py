"""
NeuroFly Overhead Vision & ArUco Autonomous Landing Tracker
============================================================
Detects ArUco markers placed on the NeuroFly drone or landing pad
and computes corrective Roll, Pitch, and Throttle commands.

Requirements:
  pip install opencv-python opencv-contrib-python numpy
"""

import cv2
import cv2.aruco as aruco
import numpy as np
import time

# Marker Configuration
ARUCO_DICT = aruco.getPredefinedDictionary(aruco.DICT_4X4_50)
TARGET_MARKER_ID = 0  # Default NeuroFly Landing Pad Marker ID
MARKER_SIZE_MM = 80.0 # Physical marker edge length in mm

def main():
    print("[NeuroFly Vision] Initializing Overhead Camera Station...")
    cap = cv2.VideoCapture(0)

    if not cap.isOpened():
        print("[Error] Could not open video device.")
        return

    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

    parameters = aruco.DetectorParameters()
    detector = aruco.ArucoDetector(ARUCO_DICT, parameters)

    print("[NeuroFly Vision] Tracking Active. Press 'q' to exit.")

    while True:
        ret, frame = cap.read()
        if not ret:
            break

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
            
            # Compute Marker Center
            marker_cx = int(np.mean(pts[:, 0]))
            marker_cy = int(np.mean(pts[:, 1]))

            # Error Vector from Target Center
            error_x = marker_cx - center_x # Positive: Drone is to the right
            error_y = center_y - marker_cy # Positive: Drone is forward

            # Proportional Control Corrections (Normalized -500 to +500)
            corr_roll = int(np.clip(-error_x * 1.5, -300, 300))
            corr_pitch = int(np.clip(error_y * 1.5, -300, 300))

            # Display Status
            cv2.line(frame, (center_x, center_y), (marker_cx, marker_cy), (0, 255, 0), 2)
            cv2.circle(frame, (marker_cx, marker_cy), 5, (0, 0, 255), -1)

            status_text = f"DX: {error_x:+d}px | DY: {error_y:+d}px -> Roll: {corr_roll:+d} Pitch: {corr_pitch:+d}"
            cv2.putText(frame, status_text, (20, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
            cv2.putText(frame, "TARGET LOCKED", (20, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
        else:
            cv2.putText(frame, "SEARCHING FOR TARGET...", (20, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)

        cv2.imshow("NeuroFly AI Vision Tracker", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
