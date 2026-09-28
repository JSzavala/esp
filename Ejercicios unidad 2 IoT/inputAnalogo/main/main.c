#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h" // Librería moderna para ADC en ESP-IDF
#include "driver/ledc.h"

#define EJEMPLO_ADC_GPIO ADC_CHANNEL_0 
#define LED_GPIO 2                     
#define LED_PWM_MAX_DUTY 4095          

void app_main(void) {
    // 1. Configurar ADC2 (en el ESP32 clásico, GPIO4 pertenece a ADC2)
    adc_oneshot_unit_handle_t adc2_handle;
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
        .clk_src = 0, // 0 asigna el reloj por defecto
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config2, &adc2_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, // Resolución de 12 bits (0 - 4095)
        .atten = ADC_ATTEN_DB_12,         // Rango de voltaje para ESP-IDF v6 (0V a ~3.3V)
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, EJEMPLO_ADC_GPIO, &config));

    ledc_timer_config_t led_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_12_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&led_timer));

    ledc_channel_config_t led_channel = {
        .gpio_num = LED_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&led_channel));

    int raw_value = 0;
    int print_counter = 0;

    while (1) {

        ESP_ERROR_CHECK(adc_oneshot_read(adc2_handle, EJEMPLO_ADC_GPIO, &raw_value));

        uint32_t duty = (uint32_t)raw_value;
        if (duty > LED_PWM_MAX_DUTY) {
            duty = LED_PWM_MAX_DUTY;
        }
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

        if (++print_counter >= 50) {
            print_counter = 0;
            printf("Valor ADC leído: %d\n", raw_value);
            printf("Voltaje ADC aproximado: %.2f V\n", raw_value * 3.3 / 4095.0);
            printf("PWM LED en D2: %u / %u\n", (unsigned)duty, (unsigned)LED_PWM_MAX_DUTY);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}