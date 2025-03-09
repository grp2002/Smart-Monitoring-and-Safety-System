# Smart-Monitoring-and-Safety-System
University of Glasgow ENG5220: Real Time Embedded Programming (2024-25) Group Project
# Project Overview  

A real-time monitoring solution for **critical and non-critical environments** using **Raspberry Pi 5**.  
This system ensures **safety, surveillance, and real-time alerts** by monitoring temperature levels and triggering alerts when predefined thresholds are exceeded.

🚀 **Developed using:** C++, Raspberry Pi GPIO, LAN-based communication, and a Camera surveillance module.

---

## 🌟 Features
✅ **Real-time temperature monitoring** using Raspberry Pi sensors  
✅ **Multi-threaded event handling** for real-time responsiveness  
✅ **Threshold alerts with buzzer, screen display, and LAN updates**  
✅ **Live camera surveillance for monitoring remote environments**  
✅ **Physical ON/OFF switch for direct control over the sensors**  
✅ **Scalable design for critical (e.g., substations) & non-critical (e.g., homes, farms) use cases**  

---

## 📌 System Architecture

### **Hardware Components**
- **Raspberry Pi 5** – Central processing unit
- **Temperature Sensors** – Reads environmental temperature
- **Buzzer** – Alerts when temperature exceeds threshold
- **Camera Module** – Provides live surveillance
- **LAN Interface** – Transmits real-time alerts to connected client applications
- **Physical Power Switch** – Allows manual activation/deactivation of sensors

### **Software Components**
- **C++ Daemon**: Periodically reads sensor data and handles event-driven actions
- **Multithreading System**: Manages different events in real-time
- **Web Interface (GitHub Pages)**: Displays project details and status updates
- **Client Applications (LAN-Based)**: Receives alerts and displays monitoring data

### **Design Scematic**
![Design Scematic](./docs/images/Smart%20Monitoring%20And%20Safety%20System%20schematic%20v1.0.png)



## 🚀 How It Works
1. **Sensors collect temperature data every 5 minutes** via GPIO.
2. The **C++ daemon processes the data** and activates event handlers based on threshold conditions.
3. Event handlers perform multiple tasks:
   - **Display alert messages on screen**
   - **Trigger a buzzer alarm**
   - **Send alerts to LAN-connected client applications**
   - **Start live video surveillance (if enabled)**
4. **Users can remotely monitor sensor readings and alerts via LAN-connected apps.**
5. The system operates autonomously with **minimal latency** using a **multi-threaded architecture**.


## 🚀 Project Management  
Track our progress on the [GitHub Project Board](https://github.com/users/grp2002/projects/3).

