#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

static TaskHandle_t gh_receiver = NULL;

void
sender (void * p_param)
{
    for (;;)
    {
        xTaskNotifyGive(gh_receiver);
#if 1
        xTaskNotifyGive(gh_receiver);
        xTaskNotifyGive(gh_receiver);
        xTaskNotifyGive(gh_receiver);
#endif
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void
receiver (void * p_param)
{
    int32_t count = 0;

    for (;;)
    {
#if 0
        count = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
#else
        count = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#endif
        printf("received notification & times %d\n", count);
    }
}

void
app_main (void)
{
    BaseType_t ret = 0;

    ret = xTaskCreate(receiver, "receiver", 2048, NULL, 2, &gh_receiver);
    assert(pdPASS == ret);
    assert(gh_receiver != NULL);

    ret = xTaskCreate(sender, "sender", 2048, NULL, 2, NULL);
    assert(pdPASS == ret);
}
