#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include "connect.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"

#define WIFI_STA_SSID   "my_station"
#define WIFI_STA_PSWD   "my_password"
#define WIFI_AP_SSID    "try_me"
#define WIFI_AP_PSWD    "password"
#define TAG         "wifi_connect"

void
task_wifi_connect (void * p_param)
{
    char p_ssid[32] = WIFI_AP_SSID;
    char p_pswd[64] = WIFI_AP_PSWD;

    // AP CONNECTION
    //
    wifi_connect_ap(p_ssid, p_pswd);

    for (int32_t idx = 0; idx < 45; ++idx)
    {
        printf("disconnecting ap in %d\n", 45 - idx);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    wifi_disconnect();

    strcpy(p_ssid, WIFI_STA_SSID);
    strcpy(p_pswd, WIFI_STA_PSWD);

    // STA CONNECTION
    //
    esp_err_t err = wifi_connect_sta(p_ssid, p_pswd, 10000);

    if (ESP_OK != err)
    {
        ESP_LOGE(TAG, "Failed to connect");
        vTaskDelete(NULL);
    }
    else
    {
        for (int32_t idx = 0; idx < 5; ++idx)
        {
            printf("disconnecting sta in %d\n", 5 - idx);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        wifi_disconnect();

        vTaskDelete(NULL);
    }
}

void
app_main (void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init();

    BaseType_t ret = xTaskCreate(task_wifi_connect, "wifi_connect", 5 * 1024, NULL, 1, NULL);
    assert(pdPASS == ret);
}
