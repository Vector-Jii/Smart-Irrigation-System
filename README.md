# 🌿 Smart Irrigation System

Automatic watering of plants 

An IoT-based automated irrigation solution designed to optimize water usage through real-time monitoring of environmental conditions. This system ensures efficient irrigation by analyzing soil moisture, weather inputs, and water levels to control a submersible pump, helping conserve water and reduce manual effort.

## 🚀 Features
- Real-time monitoring via web dashboard
- Automated pump activation based on soil moisture and rain detection
- Displays temperature, humidity, tank level, and water usage
- Remote control and monitoring capabilities

## 🧰 Hardware Components
- ESP32 / Arduino board
- Soil Moisture Sensor
- Rain Sensor
- Relay Module
- Submersible Motor (Pump)
- DHT11 and DHT22 Temperature & Humidity Sensors

## 🛠️ Software Used
- Arduino IDE

## ⚙️ How It Works
- The ESP32 collects data from connected sensors.
- When moisture falls below a defined threshold *and* no rain is detected, the pump is activated via relay.
- The system continuously monitors environmental data and updates the dashboard.
- Sensor readings include:
  - 🌱 Soil Moisture
  - 🌧️ Rain Detection
  - 💧 Water Usage
  - 🌡️ Temperature & Humidity
  - 🛢️ Tank Level

## 📡 Dashboard Interface
All metrics are displayed through an interactive web dashboard, enabling remote access and visualization.

## 📷 Basic circuit diagram
*(Include photos, circuit diagrams, or screenshots of the dashboard here)*

## 📂 Repository Structure
```bash
├── /Smart Irrigation
│   └── Soil_Irrigation_ESP32.ino
│   └── Soil_sensor_Arduino.ino
|   └── README.md
