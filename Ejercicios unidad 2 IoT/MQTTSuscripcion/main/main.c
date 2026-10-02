#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h" 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "MQTT_SUB_THINGSPEAK";

// Configuración de Credenciales de ThingSpeak (Usa tus mismos datos anteriores)
#define TS_BROKER_URI "mqtt://mqtt3.thingspeak.com"
#define TS_CLIENT_ID  "Nw8xHAA4GBMVOz05Nh4yKgI"
#define TS_USERNAME   "Nw8xHAA4GBMVOz05Nh4yKgI"
#define TS_PASSWORD   "AZJeS64uETTvWwjfDnxOY1up"
#define TS_CHANNEL_ID "3504385"

static esp_mqtt_client_handle_t client;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    char topic_buffer[128];

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Conectado al Broker. Suscribiéndose al canal...");
            
            // Construimos el tópico de suscripción para ThingSpeak
            snprintf(topic_buffer, sizeof(topic_buffer), "channels/%s/subscribe", TS_CHANNEL_ID);

            // Nos suscribimos con QoS 0
            int msg_id = esp_mqtt_client_subscribe(client, topic_buffer, 0);
            ESP_LOGI(TAG, "Suscripción enviada con éxito, ID de mensaje: %d", msg_id);
            break;

        case MQTT_EVENT_SUBCRIBED:
            ESP_LOGI(TAG, "Suscripción confirmada por el Broker. Esperando datos...");
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "¡NOTIFICACIÓN RECIBIDA!");
            
            // Imprimir el tópico de donde provienen los datos
            printf("Tópico: %.*s\r\n", event->topic_len, event->topic);
            
            // Imprimir el contenido del mensaje (Payload)
            printf("Datos recibidos: %.*s\r\n", event->data_len, event->data);
            
            // Opcional: Aquí podrías parsear la cadena (ej. buscar "field1=1")
            // y ejecutar una acción como encender un pin GPIO del ESP32-C6
            if (strncmp(event->data, "field1=1", 8) == 0) {
                ESP_LOGI(TAG, "Acción: Comando de encendido detectado.");
            }
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Desconectado del Broker.");
            break;
            
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "Error en el entorno MQTT");
            break;
            
        default:
            break;
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando ESP32-C6 en modo Suscriptor...");

    // Inicialización del almacenamiento NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Inicializar la pila de red nativa
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Conexión Wi-Fi (Recuerda configurar SSID/Password mediante idf.py menuconfig)
    ESP_ERROR_CHECK(example_connect());

    // Configuración del cliente MQTT
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = TS_BROKER_URI,
        .credentials.client_id = TS_CLIENT_ID,
        .credentials.username = TS_USERNAME,
        .credentials.authentication.password = TS_PASSWORD,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
