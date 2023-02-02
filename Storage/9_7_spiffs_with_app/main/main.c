#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_spiffs.h>
#include <stdio.h>
#include <esp_log.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/unistd.h>
#include <sys/stat.h>

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

    DIR * ph_dir = opendir("/spiffs");
    struct dirent * p_entry = NULL;

    while ((p_entry = readdir(ph_dir)) != NULL)
    {
        char fullpath[300] = {0};
        snprintf(fullpath, sizeof(fullpath), "/spiffs/%s", p_entry->d_name);
        struct stat entry_stat = {0};
        
        if (-1 == stat(fullpath, &entry_stat))
        {
            ESP_LOGE(TAG, "Error getting stats for %s", fullpath);
        }
        else
        {
            ESP_LOGI(TAG, "Full path = %s, file size = %ld", fullpath, entry_stat.st_size);
        }

        size_t total = 0;
        size_t used = 0;
        esp_spiffs_info(NULL, &total, &used);
        ESP_LOGI(TAG, "total = %d, used = %d", total, used);
    }
    

    FILE * ph_file = fopen("/spiffs/index.html", "r");

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
