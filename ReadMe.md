# Smart Monitoring: Motion-Triggered Camera System

This project uses a **PIR motion sensor** and a **Pi Camera** to create an event-driven camera system. When motion is detected, the camera feed starts. If no motion is detected for 10 seconds, the feed stops automatically.

## 📷 Motion-Activated Camera Behavior

The camera system operates based on motion detection with a timeout mechanism:

- **Motion Detection:** When the PIR sensor detects motion, it resets a countdown timer to 10 seconds.
- **Camera Activation:** If the camera is not already running, it starts capturing and displaying frames.
- **Continued Operation:** As long as motion is detected within each 10-second interval, the timer resets and the camera continues to operate.
- **Camera Deactivation:** If no motion is detected for 10 consecutive seconds, the timer expires and the camera stops to conserve resources.

This design ensures the camera remains active during motion and deactivates after a period of inactivity.

---
## 📦 Requirements

### ✅ Hardware
- Raspberry Pi (4/5 recommended)
- Pi Camera Module (libcamera-compatible)
- PIR Motion Sensor (e.g., HC-SR501)
- Jumper wires (connect PIR OUT to GPIO pin, e.g., BCM 27)

### ✅ Software Dependencies

Install required packages on your Raspberry Pi:

```bash
sudo apt update
sudo apt install -y \
    libopencv-dev \
    libcamera-dev \
    libgpiod-dev \
    cmake \
    g++
```

### 🔧 Step 1: Install libcamera2opencv
This project depends on a lightweight wrapper to connect libcamera with OpenCV.

Clone and install: \
```bash
git clone https://github.com/berndporr/libcamera2opencv.git \
cd libcamera2opencv \
cmake .. \
make \
sudo make install\
cd ..
```
### Build:
```bash
mkdir build \
cd build \
cmake .. \
make 
```
### Run:
```bash
./camera_test
```
## Architecture Overview
### Class Diagram
![image](https://github.com/user-attachments/assets/32ffcc09-2ed3-4f64-9065-1c22a97cd77c)

## Sequence Diagram
![image](https://github.com/user-attachments/assets/ce993f18-9fa9-4523-b96a-2c989dd15bc5)

---
### ⚙️ Configuration
Edit main.cpp if your motion sensor uses a different GPIO:

default setup: MotionSensor motionSensor(0, 27); // GPIO chip 0, line 27 (BCM 27)
