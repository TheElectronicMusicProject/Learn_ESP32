#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

void
task1 (void * p_param)
{
    for (;;)
    {
        printf("reading temperature from %s\n", (char *) p_param);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void
task2 (void * p_param)
{
    for (;;)
    {
        printf("reading humidity from %s\n", (char *) p_param);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void
app_main (void)
{
    BaseType_t ret = 0;

    // Core 0 è quello di default.
    //
    ret = xTaskCreatePinnedToCore(task1, "temperature_reading", 2048, "task1", 2, NULL, 0);
    assert(pdPASS == ret);
    ret = xTaskCreatePinnedToCore(task2, "humidity_reading", 2048, "task2", 2, NULL, 1);
    assert(pdPASS == ret);
}
