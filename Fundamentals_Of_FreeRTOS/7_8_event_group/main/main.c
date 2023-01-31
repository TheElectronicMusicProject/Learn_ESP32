#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <stdio.h>

EventGroupHandle_t gh_group = NULL;
const int32_t got_http = (1 << 0);
const int32_t got_ble = (1 << 1);

void
listen_for_bluetooth (void * p_param)
{
    for (;;)
    {
        xEventGroupSetBits(gh_group, got_ble);
        printf("got BLE\n");

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void
listen_for_http (void * p_param)
{
    for (;;)
    {
        xEventGroupSetBits(gh_group, got_http);
        printf("got http\n");

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void
task1 (void * p_param)
{
    for (;;)
    {
        xEventGroupWaitBits(gh_group, got_http | got_ble, pdTRUE, pdTRUE, portMAX_DELAY);
        printf("received http and BLE\n");

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void
app_main (void)
{
    BaseType_t ret = 0;
    gh_group = xEventGroupCreate();

    ret = xTaskCreate(listen_for_http, "get http", 2048, NULL, 1, NULL);
    assert(pdPASS == ret);
    ret = xTaskCreate(listen_for_bluetooth, "get BLE", 2048, NULL, 1, NULL);
    assert(pdPASS == ret);
    ret = xTaskCreate(task1, "do something with http", 2048, NULL, 1, NULL);
    assert(pdPASS == ret);
}
