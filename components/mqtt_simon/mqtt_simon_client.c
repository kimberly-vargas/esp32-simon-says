#include "mqtt_simon_client.h"


const uint8_t mqtt_server_arenalconnected_io_pem_start[]  = "-----BEGIN CERTIFICATE-----\n"
                                                            "MIIDczCCAlugAwIBAgIUbf0x5w12P16RJSEF3KOmD0KGtqkwDQYJKoZIhvcNAQEL\n"
                                                            "BQAwSTELMAkGA1UEBhMCQ1IxEzARBgNVBAgMClNvbWUtU3RhdGUxDDAKBgNVBAoM\n"
                                                            "A1RFQzEXMBUGA1UEAwwOc2ltb24tZGljZS53aW4wHhcNMjMwNTIzMTczNjQxWhcN\n"
                                                            "MjgwNTIyMTczNjQxWjBJMQswCQYDVQQGEwJDUjETMBEGA1UECAwKU29tZS1TdGF0\n"
                                                            "ZTEMMAoGA1UECgwDVEVDMRcwFQYDVQQDDA5zaW1vbi1kaWNlLndpbjCCASIwDQYJ\n"
                                                            "KoZIhvcNAQEBBQADggEPADCCAQoCggEBAK/HAxzTf+YEMVifq6YyI0WeWlfvXqE/\n"
                                                            "XddreXFw1PsTVsMcPHvpF+uJtsR0bDeKxLi6JIFYfbCerVDlwrNRPpp4Fr70Yfnj\n"
                                                            "k6Iqd89kooUDerSMxR19Ey0SWhIMSjCUpqlq8wCS/ydK5+XbM1rl8wQrtq5jIdfw\n"
                                                            "ttKgKo4obPN2r/VK/V94qM8PdM6MAImuFedQdtaOWOBNYz3PkZQOfu5oWD5bpVBh\n"
                                                            "7+Q5ckjFVA3Gu30cgFtPIc6KTiAACsCEJL5RYYRjnwhlZVTVizymk1jXmV4vtZds\n"
                                                            "EbVEEChGfJwDu6pwPORG66nUq7PeuR7Vll/qPGHlDVF8q1Qugui3c3MCAwEAAaNT\n"
                                                            "MFEwHQYDVR0OBBYEFHEIOwAMvhoI/TB8vGNamUwfxSJ8MB8GA1UdIwQYMBaAFHEI\n"
                                                            "OwAMvhoI/TB8vGNamUwfxSJ8MA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\n"
                                                            "BQADggEBAEZZLS111EOUpIGuDjBv54tXD4wVxpiJQDPiKND+nJMhMuXbidfDiAaZ\n"
                                                            "p7Bjrj+pq7aDrzbz75pjV6k6He+ML0dg/NLZhTWZpdHFc6KVDiyIOQ5dw0i79TN7\n"
                                                            "hWJVhntMyp2Hu81uYqpO0n4aVwl1nEkkGM9JaIOkpK63FVqh3ZXB2UZMBmzusGWC\n"
                                                            "2RLhM8HD17BVleKrxOxR5ZXMOX3hb7G1+2p366osUhovDWBxqwID54NEk2T9S1Qj\n"
                                                            "zXLfQsWtGY9oJF2ITxc9g2/YPiplFUNJKhWBqBaLl3X2nnnw8QZ03aPWgMhvYrMM\n"
                                                            "ax2o9huPv4u3zVB6tNSKkep/bYS5quw=\n"
                                                            "-----END CERTIFICATE-----";

const char *TAG = "MQTTS_SIMON";

QueueHandle_t queue_mqtt;
esp_mqtt_client_handle_t client;
/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "simon/secuencia", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        msg_id = esp_mqtt_client_subscribe(client, "simon/valoresEsperados", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        mqtt_msg m;
        strncpy(m.topic, event->topic, event->topic_len);
        strncpy(m.msg, event->data, event->data_len);
        xQueueSend( queue_mqtt, ( void * ) &m, ( TickType_t ) 0 );
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    queue_mqtt = xQueueCreate( 50, sizeof( mqtt_msg ) );
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = "mqtts://simon-dice.win:8883",
            .verification.certificate = (const char *)mqtt_server_arenalconnected_io_pem_start,
        },
        .credentials = {
            .username = "redes",
            .authentication.password = "123",
        }
    };

    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    /* const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = "mqtt://test.mosquitto.org",
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client); */
}

void mqtt_send_msg(mqtt_msg *m){

    ESP_LOGI(TAG, "SENDING MQTT MSG \n topic: (%s)\n msg: (%s)", m->topic, m->msg);
    esp_mqtt_client_publish(client, m->topic, m->msg, 0, 1, 0);

}