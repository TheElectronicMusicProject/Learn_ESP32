#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

#define TEST_3  1

static TaskHandle_t gh_receiver = NULL;

void
sender (void * p_param)
{
    for (;;)
    {
#if TEST_1
        xTaskNotify(gh_receiver, (1 << 0), eSetValueWithOverwrite);
        vTaskDelay(pdMS_TO_TICKS(1000));
        xTaskNotify(gh_receiver, (1 << 1), eSetValueWithOverwrite);
        vTaskDelay(pdMS_TO_TICKS(1000));
        xTaskNotify(gh_receiver, (1 << 2), eSetValueWithOverwrite);
        vTaskDelay(pdMS_TO_TICKS(1000));
        xTaskNotify(gh_receiver, (1 << 3), eSetValueWithOverwrite);
        vTaskDelay(pdMS_TO_TICKS(1000));
#elif TEST_2 | TEST_3
        xTaskNotify(gh_receiver, (1 << 0), eSetBits);
        vTaskDelay(pdMS_TO_TICKS(1000));
        xTaskNotify(gh_receiver, (1 << 1), eSetBits);
        vTaskDelay(pdMS_TO_TICKS(1000));
        xTaskNotify(gh_receiver, (1 << 2), eSetBits);
        vTaskDelay(pdMS_TO_TICKS(1000));
        xTaskNotify(gh_receiver, (1 << 3), eSetBits);
        vTaskDelay(pdMS_TO_TICKS(1000));
#endif
    }
}

void
receiver (void * p_param)
{
    uint32_t state = 0;

    for (;;)
    {
#if TEST_1 | TEST_2
        xTaskNotifyWait(0, 0, &state, portMAX_DELAY);
#elif TEST_3
        xTaskNotifyWait(0xFFFFFFFF, 0, &state, portMAX_DELAY);
#endif
        printf("received state %d\n", state);
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
