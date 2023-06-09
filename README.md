# Simon says game

This project consists of the classic game "Simon Says", played by an ESP32 microcontroller. The game user interface has been developed using Node-RED which is hosted on an Ubuntu server on AWS. In addition, to establish communication, the MQTT protocol has been implemented using mosquitto as a broker within the same server in aws.

## How to Use for windows Example

1. Install espressif
2. On esp32-simon-says/components/wifi_sta/wifi_sta.h change ESP_WIFI_SSID and ESP_WIFI_PASS to your wifi name and password
3. On the esp-idf powershell go to: cd your-path/esp32-simon-says
4. Run idf.py set-target esp32
5. Run idf.py -p PORT build flash monitor
