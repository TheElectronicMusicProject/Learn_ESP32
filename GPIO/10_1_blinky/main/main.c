#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <stdio.h>

// Vr = 3.3V - 2.75V = 0.55V
// R = 0.55V / 0.02A = 27.5ohm

#define GPIO_PIN    GPIO_NUM_2

void
app_main (void)
{
    gpio_pad_select_gpio(GPIO_PIN);
    gpio_set_direction(GPIO_PIN, GPIO_MODE_OUTPUT);
    int32_t b_is_on = 0;

    for (;;)
    {
        b_is_on = !b_is_on;
        gpio_set_level(GPIO_PIN, b_is_on);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
