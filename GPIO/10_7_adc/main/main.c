#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>
#include <stdio.h>

void
app_main (void)
{
    // Max is 4095
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_0db);

    for (;;)
    {
        int32_t val = adc1_get_raw(ADC1_CHANNEL_0);
        printf("Value is %d\n", val);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
