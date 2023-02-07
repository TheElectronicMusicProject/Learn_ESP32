#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>
#include <stdio.h>

void
app_main (void)
{
    int32_t val = 0;
    
    adc1_config_width(ADC_WIDTH_BIT_12);
    
    for (;;)
    {
        val = hall_sensor_read();
        printf("Magnetic val = %d\n", val);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
