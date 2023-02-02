#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_spiffs.h>
#include <stdio.h>
#include <esp_log.h>

#define TAG "spiffs"

void
app_main (void)
{
    esp_vfs_spiffs_conf_t config = {
        .base_path = "/spiffs",     // root directory
        .partition_label = NULL,    // use fist partition in partition table
        .max_files = 5,
        .format_if_mount_failed = true,
    };

    esp_vfs_spiffs_register(&config);

    FILE * ph_file = fopen("/spiffs/sub/data.txt", "r");

    if (ph_file != NULL)
    {
        char line[256] = {0};
        
        while (fgets(line, sizeof(line), ph_file) != NULL)
        {
            printf(line);
            vTaskDelay(1);
        }

        fclose(ph_file);
        ph_file = NULL;
    }
    else
    {
        ESP_LOGE(TAG, "Could not open file");
    }

    esp_vfs_spiffs_unregister(NULL);
}
