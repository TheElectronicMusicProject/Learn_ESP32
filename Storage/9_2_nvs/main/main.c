#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <stdint.h>
#include <esp_log.h>
#include <stdio.h>

#define TAG "NVS"

void
app_main (void)
{
    nvs_handle_t h_nvs;
    esp_err_t ret = 0;

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_ERROR_CHECK(nvs_open("store", NVS_READWRITE, &h_nvs));

    int32_t val = 0;
    ret = nvs_get_i32(h_nvs, "val", &val);

    switch (ret)
    {
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI(TAG, "Value not set yet");
        break;

        case ESP_OK:
            ESP_LOGI(TAG, "Value is %d", val);
        break;

        default:
            ESP_LOGE(TAG, "Error (%s), opening NVS handle!\n", esp_err_to_name(ret));
        break;
    }

    ++val;

    ESP_ERROR_CHECK(nvs_set_i32(h_nvs, "val", val));
    ESP_ERROR_CHECK(nvs_commit(h_nvs));

    nvs_close(h_nvs);
}
