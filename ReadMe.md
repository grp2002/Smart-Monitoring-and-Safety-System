# Smart Monitoring: Motion-Triggered Camera System

This project uses a **PIR motion sensor** and a **Pi Camera** to create an event-driven camera system. When motion is detected, the camera feed starts. If no motion is detected for 10 seconds, the feed stops automatically.

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
![53321f39-251b-4579-88a5-eb03394d8bea](https://github.com/user-attachments/assets/3a8ce5c3-e3de-442b-b94b-391c024d3b2e) 
## Sequence Diagram
![cdec9582-8dbc-476f-98f7-e4ab10ea3d49](https://github.com/user-attachments/assets/08c1c3bc-fb68-49cd-b583-bf8261cce8c6)
---
### ⚙️ Configuration
Edit main.cpp if your motion sensor uses a different GPIO:

default setup: MotionSensor motionSensor(0, 27); // GPIO chip 0, line 27 (BCM 27)
