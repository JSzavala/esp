#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_http_client.h" 

// ---------------------------------------------------------
// CONFIGURACIÓN DE LA PLATAFORMA IOT
// ---------------------------------------------------------
#define THINGSPEAK_URL "http://thingspeak.com/"
#define THINGSPEAK_API_KEY "" // <- Reemplazar con tu API Key

// Tarea de FreeRTOS para el envío periódico de información
void tarea_enviar_http(void *pvParameters) {
    while (1) {
        // 1. Capturar información (Simulación de lectura de sensor)
        float valor_sensor = 26.8; 

        // 2. Inicializar y configurar el cliente HTTP nativo
        esp_http_client_config_t config = {
            .url = THINGSPEAK_URL,
            .method = HTTP_METHOD_POST,
        };
        esp_http_client_handle_t client = esp_http_client_init(&config);

        // 3. Estructurar el cuerpo del mensaje en formato URL-Encoded
        char post_data[64];
        snprintf(post_data, sizeof(post_data), "api_key=%s&field1=%.2f", THINGSPEAK_API_KEY, valor_sensor);
        
        esp_http_client_set_post_field(client, post_data, strlen(post_data));
        esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");

        // 4. Ejecutar la petición HTTP POST hacia el servidor
        esp_err_t err = esp_http_client_perform(client);
        
        if (err == ESP_OK) {
            int status_code = esp_http_client_get_status_code(client);
            printf("[HTTP] POST Exitoso! Código de Estado: %d, Dato: %.2f\n", status_code, valor_sensor);
        } else {
            printf("[HTTP] Error en la petición: %s\n", esp_err_to_name(err));
        }

        // 5. Liberar recursos ocupados por el cliente HTTP
        esp_http_client_cleanup(client);

        // Retardo obligatorio de 15 segundos (Restricción del servidor gratuito)
        vTaskDelay(pdMS_TO_TICKS(15000));
    }
}

void app_main(void)
{
    // Inicialización del almacenamiento no volátil (NVS) y pila de red
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();

    // Conexión automática al Wi-Fi (Configurado vía menuconfig)
    printf("Estableciendo conexión Wi-Fi...\n");
    example_connect();

    // Verificación de la dirección IP asignada
    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

    if (netif != NULL) {
        esp_netif_get_ip_info(netif, &ip_info);
        printf("************************************************************\n"); 
        printf("Conectado de forma exitosa!\n");
        printf("Dirección IP asignada: " IPSTR "\n", IP2STR(&ip_info.ip)); 
        printf("************************************************************\n"); 
    }
    // Creación de la tarea independiente en segundo plano
    xTaskCreate(tarea_enviar_http, "tarea_enviar_http", 4096, NULL, 5, NULL);
}
