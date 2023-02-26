#include "toggle_led.h"
#include <stdio.h>
#include <stdlib.h>
#include <driver/gpio.h>

#define LED     GPIO_NUM_2

void
init_led (void)
{
    gpio_pad_select_gpio(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
}

void
toggle_led (bool b_is_on)
{
    gpio_set_level(LED, b_is_on);
}