## <a name="introduction">💻 Smart Home Dashboard | Thingsboard | Esp32 </a>

<img src="https://github.com/rch-goldsnaker/smart-home-dashboard-thingsboard/blob/main//banner.png" alt="Project Banner">

## 📋 <a name="table">Table of Contents</a>

1. 🤖 [Introduction](#introduction)
2. ⚙️ [Tech Stack](#tech-stack)
3. 🔋 [Features](#features)
4. 💻 [Youtube tutorial](#youtube)
5. 🤸 [Quick Start](#quick-start)
   
## <a name="introduction">🤖 Introduction</a>

Smart Home Dashboard is an advanced solution for monitoring and controlling smart homes, developed using ThingsBoard and the ESP32 as the IoT device. This system allows:

- Real-Time Monitoring: Monitor indoor and outdoor temperature, humidity, and energy consumption.
- Device Control: Turn on and adjust devices such as air conditioners, humidifiers, speakers, and vacuum cleaners. It also allows you to control the temperature of the air conditioner.
- Data Visualization: Display real-time charts of energy consumption and air conditioner temperature.
- Alarm Management: Receive alerts and consult a table when temperature or humidity values exceed predefined limits.

The Smart Home Dashboard is highly customizable, allowing users to modify and adapt the system to their specific needs.

Reference Link:

- [Thingsboard widgets development Guide](https://thingsboard.io/docs/user-guide/contribution/widgets-development/)

## <a name="tech-stack">⚙️ Tech Stack</a>

💎 Thingsboard

💎 ESP32 

## <a name="features">🔋 Features</a>

👉 ESP32 Connection

👉 Custom widgets templates

👉 Dashboard template

👉 Device Profile template

👉 Rule Chain template

## <a name="youtube">🎬 Youtube tutorial</a>

See tutorial video [here](https://youtu.be/0S_tuXnm1ps)

## <a name="quick-start">🤸 Quick Start</a>

**Prerequisites**

For Thingsboard you can create a account in Thingsboard cloud version or running locally whith docker see the video below:

- [ThingsBoard with Docker](https://youtu.be/FHFrJ4qFQ4Y?si=ZmGl3B2phoDBzkaY)

For Esp32 install library:

- thingsboard/ThingsBoard@^0.12.2
- knolleary/PubSubClient@^2.8
- arduino-libraries/ArduinoMqttClient@^0.1.8
- jandrassy/ArduinoOTA@^1.1.0
- marcoschwartz/LiquidCrystal_I2C@^1.1.4
- adafruit/DHT sensor library@^1.4.6
- adafruit/Adafruit Unified Sensor@^1.1.14

**Cloning the Repository**

```bash
git clone https://github.com/rch-goldsnaker/smart-home-dashboard-thingsboard
cd smart-home-dashboard-thingsboard
```

**Installation**

You just need import the json files and assets to Thingsboard.

**For Esp 32**

Copy the code to ArduinoIDE/Wokwi/Platformio and Upload
