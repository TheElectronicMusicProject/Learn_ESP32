#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void
app_main (void)
{
    esp_log_level_set("LOG", ESP_LOG_VERBOSE);

    ESP_LOGE("LOG", "This is an error");
    ESP_LOGW("LOG", "This is a warning");
    ESP_LOGI("LOG", "This is an info");
    ESP_LOGD("LOG", "This is a debug");
    ESP_LOGV("LOG", "This is a verbose");

    printf("a printf\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
}
