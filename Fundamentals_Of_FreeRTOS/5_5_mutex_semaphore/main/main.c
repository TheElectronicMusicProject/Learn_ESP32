#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <stdio.h>

SemaphoreHandle_t gh_mutex_bus = NULL;

static void
write_to_bus (char * message)
{
    printf("written %s\n", message);
    xSemaphoreGive(gh_mutex_bus);
}

void
task1 (void * p_param)
{
    for (;;)
    {
        printf("reading temperature from %s\n", (char *) p_param);

        if (xSemaphoreTake(gh_mutex_bus, 1000 / portTICK_PERIOD_MS))
        {
            write_to_bus("temperature is 25C");
        }
        else
        {
            printf("reading temperature timed out\n");
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void
task2 (void * p_param)
{
    for (;;)
    {
        printf("reading humidity from %s\n", (char *) p_param);

        if (xSemaphoreTake(gh_mutex_bus, 2000 / portTICK_PERIOD_MS))
        {
            write_to_bus("humidity is 50");
        }
        else
        {
            printf("reading temperature timed out\n");
        }

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void
app_main (void)
{
    BaseType_t ret = 0;
    gh_mutex_bus = xSemaphoreCreateMutex();

    ret = xTaskCreate(task1, "temperature_reading", 2048, "task1", 2, NULL);
    assert(pdPASS == ret);
    ret = xTaskCreate(task2, "humidity_reading", 2048, "task2", 2, NULL);
    assert(pdPASS == ret);
}
