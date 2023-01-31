#include <freertos/FreeRTOS.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <stdio.h>

#define TAG "Memory"

#if 0
int32_t x = 3;  // DRAM .data
int32_t y ;     // DRAM .bss
#endif

void
app_main (void)
{
    ESP_LOGI(TAG, "xPortGetFreeHeapSize %d = DRAM", xPortGetFreeHeapSize());

    int32_t dram = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    int32_t iram = heap_caps_get_free_size(MALLOC_CAP_32BIT) - dram;

    ESP_LOGI(TAG, "DRAM \t\t %d", dram);
    ESP_LOGI(TAG, "IRAM \t\t %d", iram);

    int32_t free = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    ESP_LOGI(TAG, "FREE \t\t %d", free);
}
