#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#define GPIO_OUTPUT_IO    CONFIG_GPIO_OUTPUT_1
#define TOGGLE_GPIO       GPIO_NUM_18
#define ESP_INTR_FLAG_DEFAULT 0

SemaphoreHandle_t pulse_semaphore;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    xSemaphoreGiveFromISR(pulse_semaphore, NULL);
}

static void master_task(void* arg)
{
    int cnt = 0;
    
    for (;;) {
        printf("Generating Pulse %d\n", cnt++);
        gpio_set_level(GPIO_OUTPUT_IO, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(GPIO_OUTPUT_IO, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
}

void app_main(void)
{
    gpio_config_t io_conf = {};

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_OUTPUT_IO) | (1ULL << TOGGLE_GPIO);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    pulse_semaphore = xSemaphoreCreateBinary();

    xTaskCreate(master_task, "master_task", 2048, NULL, 10, NULL);

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
