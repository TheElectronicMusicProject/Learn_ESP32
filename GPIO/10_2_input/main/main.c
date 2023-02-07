#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <stdio.h>

#define GPIO_SWITCH     GPIO_NUM_15
#define GPIO_LED        GPIO_NUM_2

void
app_main (void)
{
    gpio_pad_select_gpio(GPIO_LED);
    gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(GPIO_SWITCH);
    gpio_set_direction(GPIO_SWITCH, GPIO_MODE_INPUT);
    gpio_pulldown_en(GPIO_SWITCH);
    gpio_pullup_dis(GPIO_SWITCH);

    for (;;)
    {
        int32_t level = gpio_get_level(GPIO_SWITCH);
        gpio_set_level(GPIO_LED, level);

        vTaskDelay(1);
    }
}
