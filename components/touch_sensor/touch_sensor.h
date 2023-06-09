#ifndef __TOUCH_SENSOR_H__
#define __TOUCH_SENSOR_H__
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/touch_pad.h"
#include "esp_log.h"
#include "cJSON.h"

#define TOUCH_PAD_NO_CHANGE   (-1)
#define TOUCH_THRESH_NO_USE   (0)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

extern char JSONSequenceArray[500];
extern bool win;
extern bool loose;
extern int counter;

void _init_touch_sensor();
#endif