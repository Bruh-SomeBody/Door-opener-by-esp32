# Smart Door Web Controller

An ESP32-based web server that controls a door lock relay. It features a mobile-friendly interface for regular users and a secure admin panel for credential management.

## Features

* **Web-Based Relay Control:** Open a door directly from a web browser via a local IP address.
* **Dual-Level Authentication:** Separate login credentials for regular users and administrators.
* **Dynamic Credential Management:** Administrators can change user passwords via the web interface without modifying the code.
* **Non-Volatile Memory:** User credentials are saved to the ESP32 flash memory and persist after a reboot.
* **Auto-Reconnect:** The device automatically attempts to reconnect if the Wi-Fi connection drops.

## Hardware Requirements

* ESP32 Microcontroller
* 5V Relay Module (Connected to PIN 13)
* Electronic Door Lock

## Software & Libraries

The code is written for the Arduino IDE and requires the following standard ESP32 libraries:
* `WiFi.h`
* `WebServer.h`
* `Preferences.h`

## Configuration

Before uploading the code to your ESP32, update the following variables in the `.ino` file:

1. **Wi-Fi Settings:**
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
