#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <stdio.h>

QueueHandle_t gh_queue = NULL;

void
listen_for_http (void * p_param)
{
    int32_t count = 0;
    BaseType_t ret = 0;

    for (;;)
    {
        ++count;
        printf("received http message\n");
        ret = xQueueSendToBack(gh_queue, &count, 1000 / portTICK_PERIOD_MS);

        if (pdTRUE == ret)
        {
            printf("added message to queue\n");
        }
        else
        {
            printf("opss, FAILED!\n");
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void
task1 (void * p_param)
{
    int32_t received = 0;
    BaseType_t ret = 0;

    for (;;)
    {
        ret = xQueueReceive(gh_queue, &received, 5000 / portTICK_PERIOD_MS);

        if (pdTRUE == ret)
        {
            printf("doing something with http, %d\n", received);
        }
        else
        {
            printf("PROBLEMS\n");
        }
    }
}

void
app_main (void)
{
    BaseType_t ret = 0;
    gh_queue = xQueueCreate(3, sizeof(int32_t));

    ret = xTaskCreate(listen_for_http, "get http", 2048, NULL, 1, NULL);
    assert(pdPASS == ret);
    ret = xTaskCreate(task1, "do something with http", 2048, NULL, 1, NULL);
    assert(pdPASS == ret);
}
