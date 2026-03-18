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
   const char* ssid = "ssid";
   const char* password = "password";
3. **Admin Credentials: (Required to access the control panel)**

   ```cpp
   const char* adminUser = "adminlogin"; 
   const char* adminPass = "adminpass";

## Usage

Once the ESP32 is powered on, open the Serial Monitor (115200 baud) to find its IP address.
Main Page (User Access)

    URL: http://<ESP32_IP_ADDRESS>/

    Default Login: admin 

    Default Password: 12345678 

    Click the "Відчинити двері" (Open Door) button to trigger the relay for 1 second.

Admin Panel (Credential Management)

    URL: http://<ESP32_IP_ADDRESS>/admin 

    Login/Password: Use the hardcoded adminUser and adminPass.

    Use the form to enter a new login and password for regular users. Once saved, these details are stored in the flash memory.
