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

## How The System Works (Flow Of Information) (might change it)
The ESP32 saves in itself 3 files of HTML,JavaScript and CSS, which together form a web app.
When the ESP32 starts working it sets a WAP (WiFi Access Point) which demands a password of users. 
And When a user connects to the wifi and inputs the ESP32's IP address in a web explorer, The user gets the files of the web app that are saved in the ESP32 (Write if it's LittleFS or SPIFFS) file system.
When the user gets the web app, the web app shows the data it gets instantenously by WebSocket communication with the ESP32.
The process of collecting and saving information of the radar, from the sensors, goes the next way: 
1. The servo motor is set to be on an angle (which will be changed in the next iteration).
2. The Ultrasonic sensor sends a ultrasonic wave, and the sensor measures the time it takes the wave to come back, and by that measurement it calculates the distance the wave has past.
3. the sensor sends the distance to the ESP32, which saves the data of the angle and the distance in a JSON format, and sends the data by Web Socket communication to the User.

## Installation
### Hardware Setup
1. Connect the ultrasonic sensor to the ESP32 as per the wiring diagram.
2. Attach the servo motors for scanning.
4. Ensure all connections are secure and powered correctly.
