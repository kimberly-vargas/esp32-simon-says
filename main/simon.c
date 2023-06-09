#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/touch_pad.h"
#include "esp_log.h"
#include "wifi_sta.h"
#include "touch_sensor.h"
#include "mqtt_simon_client.h"
#include "cJSON.h"
#include "esp_mac.h"

#define GPIO_LED_PIN 2
cJSON* jsonArray = NULL;
char* username = "Anonimo";
int id = 0;

void setUserName() {
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    char* macAddress = (char*)malloc(18 * sizeof(char));
    sprintf(macAddress, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    username = macAddress;
}

void addPlayer(){
    mqtt_msg message;
    // Construir el JSON de respuesta
    cJSON* player = cJSON_CreateObject();
    //printf("USERNAME\n%s\n", username);
    cJSON_AddStringToObject(player, "name", username);
    cJSON_AddNumberToObject(player, "score", 0);
    //Enviar mensaje con el player
    strcpy(message.topic, "simon/jugadores");
    sprintf(message.msg, "%s", cJSON_Print(player));
    mqtt_send_msg(&message);
    //printf("MQTT PLAYER CONNECTED ------------------:\n%s\n", cJSON_Print(player));
    cJSON_Delete(player);
}

void task_mqtt_msg_in(void *args){
  gpio_reset_pin(GPIO_LED_PIN);
  gpio_set_direction(GPIO_LED_PIN, GPIO_MODE_OUTPUT);
  mqtt_msg m;
  cJSON* jsonSequence = NULL;
  cJSON* sequenceArray = NULL;
  cJSON* jsonExpectedValues = NULL;
  setUserName();
  addPlayer();
  while(1){
    if( xQueueReceive( queue_mqtt, &( m ), ( TickType_t ) 10 ) ) {
        if(strcmp(m.topic, "simon/secuencia") == 0){
        jsonSequence = cJSON_Parse(m.msg);
      } else if (strcmp(m.topic, "simon/valoresEsperados") == 0) {
        jsonExpectedValues = cJSON_Parse(m.msg);
        // Obtener el array dentro del atributo "sequence"
        sequenceArray = cJSON_GetObjectItem(jsonSequence, "sequence");
        if (sequenceArray == NULL || !cJSON_IsArray(sequenceArray)) {
            printf("No se encontró el array 'sequence' en el JSON.\n");
            cJSON_Delete(jsonSequence);
        } 
        // Obtener el número dentro del atributo "valoresEsperados"
        cJSON* valoresEsperadosItem = cJSON_GetObjectItem(jsonExpectedValues, "valoresEsperados");
        int valoresEsperados = 0;
        if (valoresEsperadosItem == NULL || !cJSON_IsNumber(valoresEsperadosItem)) {
            printf("No se encontró el número 'valoresEsperados' en el JSON.\n");
            cJSON_Delete(jsonExpectedValues);
        } else {
          valoresEsperados = valoresEsperadosItem->valueint;
          counter = 0;
          char valorEsperadoTexto[16];
          sprintf(valorEsperadoTexto, "%d", valoresEsperados);
        }
        //Si se hizo reset
        if(valoresEsperados == 0){
            addPlayer();
        } else {
            // Crear el arreglo de objetos JSON
            jsonArray = cJSON_CreateArray();
            // Recorrer el array hasta la posición valoresEsperados
            for (int i = 0; i < valoresEsperados; i++) {
                // Crear el objeto JSON para cada elemento
                cJSON* jsonObject = cJSON_CreateObject();
                if (jsonObject == NULL) {
                    printf("Error al crear el objeto JSON.\n");
                    cJSON_Delete(jsonArray);
                    return;
                }
                // Establecer los campos num y touched en el objeto JSON
                cJSON* sequenceItem = cJSON_GetArrayItem(sequenceArray, i);
                cJSON_AddNumberToObject(jsonObject, "num", sequenceItem->valueint);
                cJSON_AddBoolToObject(jsonObject, "touched", false);
                // Agregar el objeto JSON al arreglo
                cJSON_AddItemToArray(jsonArray, jsonObject);
            }
            // cargar el json con el texto
            strcpy(JSONSequenceArray, cJSON_Print(jsonArray));
        }
      }
     }
    if(win){
        win = false;
        // Construir el JSON de respuesta
        cJSON* response = cJSON_CreateObject();
        char id_str[20]; // Tamaño suficiente para almacenar la cadena
        sprintf(id_str, "%d", id);
        cJSON_AddStringToObject(response, "msgId", id_str);
        cJSON_AddStringToObject(response, "jugadorId", username);
        cJSON_AddBoolToObject(response, "acierto", true);
        strcpy(m.topic, "simon/confirmacionAcierto");
        sprintf(m.msg, "%s", cJSON_Print(response));
        mqtt_send_msg(&m);
        cJSON_Delete(response);
        id = id + 1;
        for (int i = 0; i < 2; i++){
            gpio_set_level(GPIO_LED_PIN, 1);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            gpio_set_level(GPIO_LED_PIN, 0);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    } else if (loose){
        gpio_set_level(GPIO_LED_PIN, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_LED_PIN, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        loose = false;
    }
    
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    connect_wifi();
    _init_touch_sensor();

    while(wifi_connected == 0){
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    mqtt_app_start();
    xTaskCreate(&task_mqtt_msg_in, "task_mqtt_msg_in", 4096, NULL, 5, NULL);
}