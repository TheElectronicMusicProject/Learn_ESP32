#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "stdint.h"

#define GPIO_LED    GPIO_NUM_2

void
app_main (void)
{
    gpio_pad_select_gpio(GPIO_LED);
    gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);
    uint32_t b_is_on = 0;

    for (;;)
    {
        b_is_on = !b_is_on;

        gpio_set_level(GPIO_LED, b_is_on);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}   /* app_main() */
