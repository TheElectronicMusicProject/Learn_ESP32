#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <stdint.h>
#include <esp_log.h>
#include <stdio.h>

#define TAG "NVS"

typedef struct cat_struct
{
    char name[30];
    int32_t age;
    int32_t id;
} cat_t;

void
app_main (void)
{
    nvs_handle_t h_nvs;
    esp_err_t ret = 0;

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(nvs_flash_init_partition("mynvs"));

    ESP_ERROR_CHECK(nvs_open_from_partition("mynvs", "cat_store", NVS_READWRITE, &h_nvs));

    nvs_stats_t nvs_stat;
    nvs_get_stats("mynvs", &nvs_stat);
    ESP_LOGI(TAG, "used: %d, free: %d, total: %d, namespace count:%d",
             nvs_stat.used_entries, nvs_stat.free_entries, nvs_stat.total_entries, nvs_stat.namespace_count);

    char cat_key[20] = {0};
    cat_t cat = {0};
    uint32_t cat_size = sizeof(cat);

    for (int32_t idx = 0; idx < 5; ++idx)
    {
        snprintf(cat_key, sizeof(cat_key), "cat_%d", idx);
        ret = nvs_get_blob(h_nvs, cat_key, (void *) &cat, &cat_size);

        switch (ret)
        {
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "Value not set yet");
            break;

            case ESP_OK:
                ESP_LOGI(TAG, "Cat name: %s, age %d, id %d", cat.name, cat.age, cat.id);
            break;

            default:
                ESP_LOGE(TAG, "Error (%s), opening NVS handle!\n", esp_err_to_name(ret));
            break;
        }
    }

    for (int32_t idx = 0; idx < 5; ++idx)
    {
        cat_t new_cat = {0};
        snprintf(new_cat.name, sizeof(new_cat), "Mr cat_%d", idx);
        new_cat.age = idx + 2;
        new_cat.id = idx;

        snprintf(cat_key, sizeof(cat_key), "cat_%d", idx);
        ESP_ERROR_CHECK(nvs_set_blob(h_nvs, cat_key, (void *) &new_cat, sizeof(new_cat)));
        ESP_ERROR_CHECK(nvs_commit(h_nvs));
    }

    nvs_close(h_nvs);
}
