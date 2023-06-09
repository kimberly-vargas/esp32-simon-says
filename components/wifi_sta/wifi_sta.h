#ifndef __WIFI_STA_H__
#define __WIFI_STA_H__
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"


#define ESP_WIFI_SSID      "iPhone de Kim"
#define ESP_WIFI_PASS      "kiki1234"
#define ESP_MAXIMUM_RETRY  5

#define TAG_WIFI           "wifi station"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

extern uint8_t wifi_connected;

void connect_wifi();
#endif