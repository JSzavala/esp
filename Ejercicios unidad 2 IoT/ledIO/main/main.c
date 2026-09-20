#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"

#define LED_PIN 4
#define BTN_PIN 2

void app_main(void)
{
  int led_state = 0;
  gpio_reset_pin(LED_PIN);
  gpio_reset_pin(BTN_PIN);
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(BTN_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(BTN_PIN, GPIO_PULLDOWN_ONLY);
  gpio_set_level(LED_PIN, led_state);
  while (1)
  {
    led_state = gpio_get_level(BTN_PIN);
    gpio_set_level(LED_PIN, led_state);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}