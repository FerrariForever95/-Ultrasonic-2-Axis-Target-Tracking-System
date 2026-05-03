# Ultrasonic 2-Axis Target Tracking System

A real-time target tracking system using an ultrasonic sensor, dual-axis servo mechanism, and PIR-based motion detection. The system scans its environment, locks onto the nearest object, aligns vertically, and continuously tracks movement with stable control logic.

---

## Features

- 2-axis tracking (pan + tilt)
- Automatic target acquisition via wide scan
- Vertical alignment using tilt scan (head targeting logic)
- Continuous tracking with noise-resistant filtering
- PIR-based motion detection for automatic re-acquisition
- Laser indication (blinks on lock, steady during tracking)
- Configurable parameters for tuning behavior

---

## How It Works

1. **Initialization**
   - PIR sensor calibrates
   - System prepares servos and sensors

2. **Scan Phase**
   - Pan servo sweeps across defined range
   - Closest valid object is selected
   - Tilt servo scans upward to align with top of object

3. **Lock Phase**
   - System locks onto target position
   - Laser blinks once to indicate lock

4. **Tracking Phase**
   - Continuously measures distance
   - Adjusts pan to follow movement
   - Ignores invalid ultrasonic readings (0 / max range)

5. **Reacquisition**
   - Triggered if:
     - Target is lost
     - PIR detects motion
   - System resets tilt and rescans

---

## Hardware Used

- Ultrasonic Sensor (HC-SR04 or equivalent)
- 2x Servo Motors (Pan + Tilt)
- PIR Motion Sensor
- Microcontroller (Arduino / ESP32 compatible)
- Laser module (optional for visual feedback)

---

## Pin Configuration

| Component        | Pin |
|----------------|-----|
| Ultrasonic TRIG| 12  |
| Ultrasonic ECHO| 11  |
| Pan Servo      | 9   |
| Tilt Servo     | 10  |
| PIR Sensor     | 2   |
| Laser          | 13  |

---

## Key Parameters

All behavior can be tuned using `#define`:

- `SCAN_STEP` → scan resolution
- `TRACK_THRESHOLD` → sensitivity to movement
- `SERVO_SETTLE` → stability delay
- `TILT_TOP` → maximum upward angle
- `CONFIRM_COUNT` → noise filtering strength
- `LOST_COUNT_LIMIT` → when to trigger rescan

---

## Design Notes

- Uses **single-measurement approach** for stability instead of aggressive filtering
- Ignores invalid readings rather than reacting to them
- Separates scan, lock, and tracking phases for predictable behavior
- Avoids continuous tilt tracking to prevent instability

---

## Limitations

- Ultrasonic sensors have limited angular resolution
- Performance depends on surface reflectivity
- Not suitable for fast-moving or small targets
- No object identification (distance-based only)

---

## Future Improvements

- Camera-based tracking (ESP32-CAM / OpenCV)
- Smooth servo interpolation (non-blocking control)
- Multi-sensor fusion (IR + ultrasonic)
- Target prioritization logic

---

## Author

Designed and implemented as a real-time embedded tracking system focused on stability and deterministic behavior.
