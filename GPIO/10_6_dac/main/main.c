#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/dac.h>
#include <stdio.h>

void
app_main (void)
{
    dac_output_enable(DAC_CHANNEL_1);

    for (uint8_t idx = 0; idx < 256; ++idx)
    {
        dac_output_voltage(DAC_CHANNEL_1, idx); // from 0 to 255
        vTaskDelay(1);
    }
}
