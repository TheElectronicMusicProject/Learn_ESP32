#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define TAG "Memory"
#define TEST_3  1

#if TEST_3
static void
task_a (void * p_param)
{
    int32_t stack_mem = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI(TAG, "stack space = %d", stack_mem);

    char buffer[6000];
    memset(buffer, 1, sizeof(buffer));
    buffer[0] += 1;

    for (;;)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
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

#if TEST_1
    char buffer[163840];
    memset(buffer, 1, sizeof(buffer));
    buffer[0] += 1;
#elif TEST_2
    int32_t stack_mem = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI(TAG, "stack space = %d", stack_mem);
    char buffer[stack_mem];
    memset(buffer, 1, sizeof(buffer));
    buffer[0] += 1;
#elif TEST_3
    int32_t stack_mem = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI(TAG, "stack space = %d", stack_mem);
    xTaskCreate(task_a, "a task", 8000, NULL, 1, NULL);
#endif
}
