# Radar Project

## Overview
This project is a functional radar system built using the **ESP32-WROVER microcontroller**, ultrasonic sensors, servo motors, and other electronic components. It collects data from the radar system and securely transmits it to a web app for visualization and control.

## Features
- Real-time radar data collection using an ultrasonic sensor.
- Servo motor control for scanning the environment.
- Secure communication through a Wi-Fi connection with password protection.
- User-friendly web app for:
  - Displaying radar output.
  - Controlling additional features of the radar system.

## Components
### Hardware
- ESP32-WROVER
- Ultrasonic Sensor (HC-SR04P)
- SG90 Servo Motors
- Breadboard, resistors, wires, and power supply.

### Software
- Programming Languages:
  - Arduino-style C++: For ESP32 firmware (based on the Arduino framework).
  - HTML/CSS/JavaScript: For the web application.
- Frameworks and SDKs:
  - Arduino Framework: Simplifies ESP32 programming using a C++-based API.
  - Protocols: WebSockets for real-time communication.

## Installation
### Hardware Setup
1. Connect the ultrasonic sensor to the ESP32 as per the wiring diagram.
2. Attach the servo motors for scanning.
4. Ensure all connections are secure and powered correctly.

### Software Setup
1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/radar-project.git
   cd radar-project
