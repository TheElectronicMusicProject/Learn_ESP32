#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <esp_log.h>

void
app_main (void)
{
    int count = 0;

    for (;;)
    {
        ESP_LOGI("QEMU", "running QEMU count %d", count++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
