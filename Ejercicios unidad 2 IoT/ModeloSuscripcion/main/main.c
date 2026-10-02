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
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "MQTT_THINGSPEAK";

// Configuración de Credenciales de ThingSpeak
#define TS_BROKER_URI "mqtt://mqtt3.thingspeak.com"
#define TS_CLIENT_ID  ""
#define TS_USERNAME   ""
#define TS_PASSWORD   ""
#define TS_CHANNEL_ID ""
static esp_mqtt_client_handle_t client;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Conectado exitosamente al Broker de ThingSpeak");
            
            // Creamos el payload en formato que acepta ThingSpeak (URL encoded)
            char payload[64];
            int valor_simulado = 25; // Ejemplo: Temperatura de 25°C
            snprintf(payload, sizeof(payload), "field1=%d&status=MQTTPublish", valor_simulado);

            // Construimos el tópico correcto
            char topic[64];
            snprintf(topic, sizeof(topic), "channels/%s/publish", TS_CHANNEL_ID);

            // Publicamos: QoS=0, Retain=0
            int msg_id = esp_mqtt_client_publish(client, topic, payload, 0, 0, 0);
            ESP_LOGI(TAG, "Mensaje publicado con ID: %d", msg_id);
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Desconectado del Broker. Intentando reconexión...");
            break;
            
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "Error en el evento MQTT");
            break;
            
        default:
            break;
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando ESP32-C6...");

    // Inicializar memoria NVS requerida por el Wi-Fi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Inicializar interfaz de red y bucle de eventos
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Conexión Wi-Fi integrada de ESP-IDF (Configurar mediante idf.py menuconfig)
    ESP_ERROR_CHECK(example_connect());

    // Configuración del cliente MQTT de Espressif
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
