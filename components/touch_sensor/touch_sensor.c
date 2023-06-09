#include "touch_sensor.h"
#include "cJSON.h"

/*
  Read values sensed at all available touch pads.
 Print out values in a loop on a serial monitor.
 */

char JSONSequenceArray[500] = " ";
int counter = 0;
bool win = false;
bool loose = false;
int sensores[4] = {4, 5, 6, 7};

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "cJSON.h"

bool checkAllTouched(const cJSON* jsonArray) {
    // Verificar si todos los objetos tienen touched=true
    bool allTrue = true;
    cJSON* item = NULL;
    cJSON_ArrayForEach(item, jsonArray) {
        cJSON* touchedObj = cJSON_GetObjectItem(item, "touched");
        if (touchedObj == NULL || !cJSON_IsTrue(touchedObj)) {
            allTrue = false;
            break;
        }
    }

    // Devolver el resultado
    return allTrue;
}

void updateJSON(const char* json, int num) {
    // Convertir el char a un objeto cJSON
    cJSON* root = cJSON_Parse(json);
    if (root == NULL) {
        fprintf(stderr, "Error al convertir el JSON.\n");
        return;
    }
    // Obtener el arreglo de objetos cJSON
    cJSON* jsonArray = root;
    // Buscar y actualizar el objeto que coincida con el número dado
    cJSON* item = cJSON_GetArrayItem(jsonArray, counter); 
    cJSON* numObj = cJSON_GetObjectItem(item, "num");
    if (numObj != NULL && numObj->valueint == num) { 
        cJSON* touchedObj = cJSON_GetObjectItem(item, "touched");
        if (touchedObj != NULL) {
            cJSON* jsonNewObject = cJSON_CreateObject();
            cJSON_AddNumberToObject(jsonNewObject, "num", num);
            cJSON_AddBoolToObject(jsonNewObject, "touched", true);
            cJSON_ReplaceItemInArray(jsonArray, counter, jsonNewObject);
            counter = counter + 1;
        }
        // Convertir el objeto cJSON a un string JSON y guardarlo en la variable global
        strcpy(JSONSequenceArray, cJSON_Print(root));
        // Verificar si todos los objetos están en true
        bool result = checkAllTouched(jsonArray);
        if (result) {
            //Todos los objetos tienen touched = true
            counter = 0;
            win = true;
        }
    } else {
        loose = true;
        counter = 0;
    }
    // Liberar la memoria
    cJSON_Delete(root);
    return;
}


static void tp_read_task(void *pvParameter)
{
    uint16_t touch_value;
    uint16_t touch_filter_value;
    int touched_pin = -1;
    bool pinTouched = false;

    printf("Touch Sensor filter mode read, the output format is: \nTouchpad num:[raw data, filtered data]\n\n");

    while (1) {
        for (int i = 0; i < 4; i++) {
            touch_pad_read_raw_data(sensores[i], &touch_value);
            touch_pad_read_filtered(sensores[i], &touch_filter_value);
            if (touch_value < 700 && pinTouched == false) {
                pinTouched = true;
                touched_pin = sensores[i]; // Almacena el número de pin si es tocado
            } else if (touch_value > 1300 && pinTouched == true && touched_pin == sensores[i]){
                pinTouched = false;
            }
            vTaskDelay(5 / portTICK_PERIOD_MS);
        }
        //printf("\n");
        if (touched_pin != -1 && pinTouched == false) {
            printf("El pin T%d fue tocado\n", touched_pin);
            if(!loose){
                updateJSON(JSONSequenceArray, touched_pin - 4);
            }
            touched_pin = -1; // Restablece la variable para el próximo ciclo
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

static void tp_touch_pad_init(void)
{
    for (int i = 0; i< 4;i++) {
        touch_pad_config(sensores[i], TOUCH_THRESH_NO_USE);
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

void _init_touch_sensor(void)
{
    // Initialize touch pad peripheral.
    // The default fsm mode is software trigger mode.
    ESP_ERROR_CHECK(touch_pad_init());
    // Set reference voltage for charging/discharging
    // In this case, the high reference valtage will be 2.7V - 1V = 1.7V
    // The low reference voltage will be 0.5
    // The larger the range, the larger the pulse count value.
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    tp_touch_pad_init();

    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);

    // Start task to read values sensed by pads
    xTaskCreate(&tp_read_task, "touch_pad_read_task", 4096, NULL, 5, NULL);
}
