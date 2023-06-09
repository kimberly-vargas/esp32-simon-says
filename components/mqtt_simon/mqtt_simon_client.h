
#ifndef __MQTT_SIMON_CLIENT_H__
#define __MQTT_SIMON_CLIENT_H__
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "esp_event.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include <sys/param.h>

typedef struct
{
    char topic[32];
    char msg[150];
} mqtt_msg;

extern QueueHandle_t queue_mqtt;

void mqtt_app_start(void);

void mqtt_send_msg(mqtt_msg *m);
#endif