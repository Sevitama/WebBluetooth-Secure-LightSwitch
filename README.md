# WebBluetooth ESP32 Example

Introduction
----

This repository contains an example webpage that uses WebBluetooth to connect to an BLE device. With this example, you can control the LED on the ESP32 using a web browser and subscribe to two different button press notifications. One of which requires autentication and encryption. When subscribing to the secured button press notifications, the BLE connection is encrypted using Numeric Comparison, ensuring a secure connection. The repository also includes the Arduino code for the ESP32, making it easy to get started.

Getting started
----
Because WebBluetooth is still considered experimental (state January, 2024), you'll need to activate it in the settings. Navigate to this URL: `chrome://flags/#enable-web-bluetooth-new-permissions-backend` and set it to enabled. Then restart your browser.

To use this example with an ESP32, flash the `esp_nimble_advanced.ino` file onto your ESP32 using Arduino. Navigate to the [GitHub page](sevitama.github.io/WebBluetooth-Secure-LightSwitch/) of this repository and connect with your ESP32. From there, you can try out the various features and explore the capabilities of WebBluetooth.
