#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "SISTEMA";

#define BLINK_GPIO GPIO_NUM_32  // El LED integrado (RGB) del ESP32-C6 suele usar el GPIO 8

// TAREA 1: Parpadeo de un LED (Simula control de periféricos)
void vTareaLed(void *pvParameters) {
    // Configura el GPIO del LED como salida
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    uint8_t estado_led = 0;

    while (1) {
        estado_led = !estado_led;
        gpio_set_level(BLINK_GPIO, estado_led);
        ESP_LOGI("TAREA_LED", "Led cambiado a: %d", estado_led);

        // Se bloquea por 500ms, permitiendo que la Tarea 2 y el sistema ejecuten código
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// TAREA 2: Lectura de un sensor ficticio (Simula adquisición de datos)
void vTareaSensor(void *pvParameters) {
    int contador_lecturas = 0;

    while (1) {
        contador_lecturas++;
        ESP_LOGW("TAREA_SENSOR", "Leyendo sensor... Muestra #%d", contador_lecturas);

        // Se bloquea por 1000ms. Al tener tiempos de delay diferentes a la Tarea 1,
        // el planificador alternará perfectamente entre ambas sin saturar el núcleo.
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando configuración de tareas en el único núcleo del ESP32-C6...");

    // Creamos la Tarea 1 (Prioridad 2)
    xTaskCreate(
        vTareaLed,          // Función que ejecuta la tarea
        "ControlLed",       // Nombre identificativo (para debug)
        2048,               // Stack size en bytes
        NULL,               // Parámetros de entrada
        2,                  // Prioridad
        NULL                // Descriptor (no lo necesitamos aquí)
    );

    // Creamos la Tarea 2 (Misma Prioridad 2)
    xTaskCreate(
        vTareaSensor,       // Función que ejecuta la tarea
        "LecturaSensor",    // Nombre identificativo
        2048,               // Stack size en bytes
        NULL,               // Parámetros de entrada
        2,                  // Prioridad igual para activar el Time-slicing
        NULL                // Descriptor
    );

    ESP_LOGI(TAG, "Tareas creadas. app_main entra en bucle de bajo consumo.");

    // app_main se queda como un monitor de fondo. 
    // Como tiene prioridad 1 (menor que las tareas), solo correrá cada 5 segundos.
    while (1) {
        ESP_LOGI(TAG, "[Alerta] El sistema sigue vivo y estable.");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

