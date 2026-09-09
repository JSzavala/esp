#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "SISTEMA";

// 1. Definimos la estructura de los datos que vamos a enviar
typedef struct {
    int id_lectura;
    float temperatura;
} datos_sensor_t;

// 2. Declaramos el manejador de la cola
QueueHandle_t cola_sensor = NULL;

// TAREA 1: Productora (Mide el sensor y envía el dato a la cola)
void vTareaProductora(void *pvParameters) {
    int contador = 0;
    datos_sensor_t muestra;

    while (1) {
        contador++;
        // Simulamos la lectura de un sensor
        muestra.id_lectura = contador;
        muestra.temperatura = 22.5f + (contador % 5); // Simulación

        ESP_LOGI("PRODUCTOR", "Enviando lectura #%d a la cola...", muestra.id_lectura);

        // Enviamos el dato a la cola. Esperamos hasta 0 ticks si está llena (no se bloquea al enviar).
        if (xQueueSend(cola_sensor, &muestra, 0) != pdPASS) {
            ESP_LOGE("PRODUCTOR", "¡Error! La cola está llena.");
        }

        // Esta tarea sí usa delay para controlar CADA CUÁNTO mide el sensor (cada 2 segundos)
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// TAREA 2: Consumidora (Espera los datos y los procesa)
void vTareaConsumidora(void *pvParameters) {
    datos_sensor_t dato_recibido;

    while (1) {
        // AQUÍ ESTÁ EL TRUCO: portMAX_DELAY le dice a FreeRTOS:
        // "Duerme esta tarea indefinidamente. No consumas nada de CPU hasta que haya un dato".
        // No necesitas poner un vTaskDelay() abajo.
        if (xQueueReceive(cola_sensor, &dato_recibido, portMAX_DELAY) == pdPASS) {
            
            // En cuanto llega un dato, FreeRTOS despierta esta tarea inmediatamente
            ESP_LOGW("CONSUMIDOR", "¡Dato recibido! ID: %d, Temp: %.2f °C", 
                     dato_recibido.id_lectura, 
                     dato_recibido.temperatura);
            // Al terminar el ciclo, regresa al xQueueReceive y se vuelve a dormir.
        }
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando sistema de colas en un solo núcleo...");

    // 3. Creamos la cola antes de lanzar las tareas
    // Tendrá capacidad para almacenar hasta 5 estructuras del tipo 'datos_sensor_t'
    cola_sensor = xQueueCreate(5, sizeof(datos_sensor_t));

    if (cola_sensor != NULL) {
        
        // Creamos la tarea consumidora con prioridad 2
        xTaskCreate(vTareaConsumidora, "Consumidor", 2048, NULL, 2, NULL);

        // Creamos la tarea productora con la misma prioridad 2
        xTaskCreate(vTareaProductora, "Productor", 2048, NULL, 2, NULL);
        
    } else {
        ESP_LOGE(TAG, "Error al crear la cola.");
    }
}
