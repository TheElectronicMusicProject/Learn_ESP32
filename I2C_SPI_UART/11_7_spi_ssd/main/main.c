#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/dirent.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "esp_err.h"
#include "sdmmc_cmd.h"
#include "driver/gpio.h"

#define PIN_NUM_MISO    19
#define PIN_NUM_MOSI    21
#define PIN_NUM_CLK     18
#define PIN_NUM_CS      5
#define MAX_BUFSIZE     16384
#define TAG             "spi"

static void
list_dir (const char * p_dir_name)
{
    DIR * ph_dir = opendir(p_dir_name);

    if (NULL == p_dir_name)
    {
        ESP_LOGE(TAG, "cannot open dir %s", p_dir_name);
        return;
    }

    struct dirent * p_entry = {0};

    while (NULL != (p_entry = readdir(ph_dir)))
    {
        char fullpath[300] = {0};
        struct stat entry_stat = {0};

        sprintf(fullpath, "%s/%s", p_dir_name, p_entry->d_name);

        if (-1 == stat(fullpath, &entry_stat))
        {
            ESP_LOGE(TAG, "error getting stats for %s", fullpath);
        }
        else
        {
            if (DT_DIR != p_entry->d_type)
            {
                ESP_LOGI(TAG, "file full path = %s, file size = %ld", fullpath, entry_stat.st_size);
            }
            else
            {
                ESP_LOGI(TAG, "dir full path = %s, file size = %ld", fullpath, entry_stat.st_size);
                list_dir(fullpath);
            }
        }
    }
}

void
app_main (void)
{
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = PIN_NUM_MISO;
    slot_config.gpio_mosi = PIN_NUM_MOSI;
    slot_config.gpio_sck = PIN_NUM_CLK;
    slot_config.gpio_cs = PIN_NUM_CS;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5
    };

    gpio_pad_select_gpio(GPIO_NUM_13);
    gpio_set_direction(GPIO_NUM_13, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_13, GPIO_PULLUP_ONLY);

    while (1 == gpio_get_level(GPIO_NUM_13))
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    sdmmc_card_t * p_card = NULL;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &p_card);

    if (ESP_OK != ret)
    {
        if (ESP_FAIL == ret)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card: %s", esp_err_to_name(ret));
        }

        return;
    }

    // Print info about the card.
    //
    sdmmc_card_print_info(stdout, p_card);

    ESP_LOGI(TAG, "Opening a file");

    FILE * ph_file = fopen("/sdcard/hello.txt", "w");

    if (NULL == ph_file)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }

    fprintf(ph_file, "Hello %s\n", p_card->cid.name);
    fclose(ph_file);
    ph_file = NULL;

    ESP_LOGI(TAG, "File written");

    struct stat stats = {0};

    if (0 == stat("/sdcard/foo.txt", &stats))
    {
        unlink("/sdcard/foo.txt");
    }

    ESP_LOGI(TAG, "Renaming file");

    if (0 != rename("/sdcard/hello.txt", "/sdcard/foo.txt"))
    {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    ESP_LOGI(TAG, "Reading file");

    ph_file = fopen("/sdcard/foo.txt", "r");

    if (NULL == ph_file)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    char line[64] = {0};
    fgets(line, sizeof(line), ph_file);
    fclose(ph_file);
    ph_file = NULL;

    char * p_pos = strchr(line, '\n');

    if (p_pos)
    {
        *p_pos = '\0';
    }

    ESP_LOGI(TAG, "Read from file %s", line);

    list_dir("/sdcard");

    esp_vfs_fat_sdcard_unmount("/sdcard", p_card);
    ESP_LOGI(TAG, "Card unmounted");
}
