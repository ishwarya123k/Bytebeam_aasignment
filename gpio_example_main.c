#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#define GPIO_INPUT_IO     CONFIG_GPIO_INPUT_1
#define ESP_INTR_FLAG_DEFAULT 0

SemaphoreHandle_t pulse_semaphore;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    xSemaphoreGiveFromISR(pulse_semaphore, NULL);
}

static void slave_task(void* arg)
{
    uint32_t pulse_count = 0;
    bool led_status = false;

    for (;;) {
        if (xSemaphoreTake(pulse_semaphore, portMAX_DELAY) == pdTRUE) {
            pulse_count++;
            printf("Pulse Count: %ld\n", pulse_count);

            // Toggle LED on pulse trigger
            led_status = !led_status;
            gpio_set_level(GPIO_NUM_18, led_status);

            printf("LED Status: %s\n", led_status ? "ON" : "OFF");
        }
    }
}

void app_main(void)
{
    gpio_config_t io_conf = {};

    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_INPUT_IO);
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    pulse_semaphore = xSemaphoreCreateBinary();

    xTaskCreate(slave_task, "slave_task", 2048, NULL, 10, NULL);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INPUT_IO, gpio_isr_handler, (void*) GPIO_INPUT_IO);

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
