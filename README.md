# Multi-Mode Arduino Robot 

This project implements a **multi-mode robotic control system** using Arduino, supporting manual control, line following, obstacle avoidance, hybrid navigation, trigger-based motion, gyro-stabilized driving, record/playback of paths, and **voice control via Google Text-to-Speech (G-TTS)** using a mobile device.

The robot uses a **2-motor differential drive** architecture with ultrasonic sensing, IR line sensors, MPU6050 gyro, servo scanner, servo claw, buzzer feedback, wireless communication, and regulated power distribution.

---

## Features

### 1. Manual Mode (`M`)
- Direct serial / Bluetooth control  
- Forward / Backward / Left / Right / Stop  
- Claw open and close  

### 2. Line Follower Mode (`X`)
- Dual IR sensors  
- Follows a high-contrast line  
- Stops when line is lost  

### 3. Obstacle Avoidance Mode (`O`)
- Ultrasonic sensor with scanning servo  
- Chooses the clearer path dynamically  

### 4. Hybrid Mode (`H`)
- Default forward motion  
- Automatically avoids obstacles within a safe distance  
- Returns to forward drive after avoidance  

### 5. Trigger Mode (`T`)
Distance-based behavior:
- **15–50 cm** → Move forward  
- **≤15 cm** → Move backward  
- **Outside range** → Stop  

### 6. Gyro-Stabilized Mode (`Y`)
- Uses MPU6050 yaw data  
- Maintains straight-line motion  
- Corrects motor imbalance and surface drift  

### 7. Record & Play Mode
- Records manual commands with timing  
- Stores up to **80 steps**  
- Autonomous playback of the recorded path  

### 8. Voice Control (Google Text-to-Speech)
- Control the robot using **spoken commands from a smartphone**  
- Google Speech-to-Text converts voice into text  
- Commands are sent over **Bluetooth (HC-05)** or USB serial  
- Arduino parses received characters as control commands  

### 9. Audible Feedback
- Buzzer confirmation on:
  - Mode change  
  - Start/stop recording  
  - Playback start  

---

## Hardware Requirements

- Arduino Uno / Nano  
- 2 × DC motors  
- Motor driver (L298N or equivalent)  
- **MPU6050 gyroscope module**  
- Ultrasonic sensor (HC-SR04)  
- 2 × IR line sensors  
- 1 × Servo motor (scanner)  
- 1 × Servo motor (claw)  
- **HC-05 Bluetooth module**  
- Buzzer  
- **Buck converter (step-down voltage regulator)**  
- **Prototype shield with mini breadboard**  
- External battery (Li-ion / LiPo recommended)  
- Jumper wires and robot wheels/chassis  

---

## Power Management

### Buck Converter
- Steps down battery voltage (7.4–12 V) to **stable 5 V**  
- Powers Arduino, sensors, MPU6050, HC-05, and servos  
- Prevents brown-outs and random resets caused by motors  

### Prototype Shield with Breadboard
- Provides clean and compact wiring  
- Ideal for:
  - Buck converter mounting  
  - HC-05 power and signal routing  
  - Sensor power distribution  
  - Buzzer and servo connections  
- Improves reliability compared to loose jumper connections  

---

## Pin Configuration

### Motors
| Function | Pin |
|--------|-----|
| Left Enable | D5 |
| Right Enable | D6 |
| Left IN1 | D8 |
| Left IN2 | D9 |
| Right IN1 | D10 |
| Right IN2 | D11 |

### Line & Distance Sensors
| Sensor | Pin |
|------|-----|
| IR Left | A2 |
| IR Right | A3 |
| Ultrasonic Trigger | A0 |
| Ultrasonic Echo | A1 |

### Other Components
| Component | Pin |
|---------|-----|
| Claw Servo | D4 |
| Scan Servo | D3 |
| Buzzer | D12 |
| HC-05 TX | D0 (Arduino RX) |
| HC-05 RX | D1 (Arduino TX, via divider) |

---

## MPU6050 Pin Configuration (I²C)

| MPU6050 Pin | Arduino Uno / Nano |
|------------|--------------------|
| VCC | 5V *(or 3.3V if module requires)* |
| GND | GND |
| SDA | A4 |
| SCL | A5 |
| INT | Not used *(optional)* |
| AD0 | GND *(I²C address 0x68)* |

**Notes:**
- MPU6050 uses **I²C communication**
- Keep SDA and SCL wires short for reliable communication
- Place the MPU6050 away from motors to reduce vibration and noise

---

## Serial Command Set

### Mode Selection
| Command | Mode |
|-------|------|
| `M` | Manual |
| `X` | Line Follower |
| `O` | Obstacle Avoid |
| `H` | Hybrid |
| `T` | Trigger |
| `Y` | Gyro Drive |

### Movement Commands
| Command | Action |
|-------|--------|
| `F` | Forward |
| `B` | Backward |
| `L` | Left |
| `R` | Right |
| `S` | Stop |

### Claw Control
| Command | Action |
|-------|--------|
| `G` | Open Claw |
| `g` | Close Claw |

### Record & Playback
| Command | Action |
|-------|--------|
| `Z` | Start Recording |
| `E` | Stop Recording |
| `P` | Play Recorded Path |

---
## Adjustable Parameters – Tuning Guide

### Motor RPM
- Higher RPM motors may require **lower `MOTOR_SPEED` values** to maintain stability and control.
- Excessively high speed can cause overshoot in turns and poor line tracking.

### Robot Mass
- Heavier robots may need:
  - Higher speed or torque settings
  - Longer delays during obstacle avoidance maneuvers
- Lightweight robots usually respond better to lower correction values.

### Battery Voltage
- Higher battery voltage increases motor speed and acceleration.
- Always **retune speed and gyro correction constants** after changing battery type or voltage.

### Surface Friction and Sensor Placement
- Low-friction surfaces require gentler steering corrections.
- Poorly aligned IR or ultrasonic sensors require:
  - Adjusted distance thresholds
  - Reduced correction aggressiveness

---

## Libraries Required

- `Servo`
- `Wire`
- `MPU6050_light`

Install all required libraries using the **Arduino Library Manager**.

---

## Notes

- Ensure a **common ground** across:
  - Arduino
  - Motor driver
  - Buck converter
  - All sensors and modules

- Keep the robot **stationary during MPU6050 calibration** at startup for accurate gyro offsets.

- Ultrasonic timeout is configured to **prevent blocking behavior** and improve responsiveness.

### Voice Control Reliability Depends On:
- Mobile application quality  
- Microphone clarity  
- Ambient noise levels  

---
## License

Open-source for learning, prototyping, project expos and competition use.


## Adjustable Parameters

```cpp
#define MOTOR_SPEED 203
#define GYRO_CORRECT 3
#define HYBRID_SAFE_DISTANCE 25
#define T_MIN_DISTANCE 15
#define T_MAX_DISTANCE 50

