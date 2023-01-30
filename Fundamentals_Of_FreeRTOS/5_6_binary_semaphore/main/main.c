#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <stdio.h>

SemaphoreHandle_t gh_bin_semaphore = NULL;

void
listen_for_http (void * p_param)
{
    for (;;)
    {
        printf("received http message\n");
        xSemaphoreGive(gh_bin_semaphore);
        printf("processed http message\n");

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void
task1 (void * p_param)
{
    for (;;)
    {
        xSemaphoreTake(gh_bin_semaphore, portMAX_DELAY);
        printf("doing something with http\n");
    }
}

void
app_main (void)
{
    BaseType_t ret = 0;

    gh_bin_semaphore = xSemaphoreCreateBinary();

    ret = xTaskCreate(listen_for_http, "get http", 2048, NULL, 2, NULL);
    assert(pdPASS == ret);
    ret = xTaskCreate(task1, "do something with http", 2048, NULL, 1, NULL);
    assert(pdPASS == ret);
}
