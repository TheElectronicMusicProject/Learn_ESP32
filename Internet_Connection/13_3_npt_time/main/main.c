#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>
#include <esp_sntp.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_err.h>
#include "protocol_examples_common.h"
#include <stdio.h>

#define TAG     "npt_time"

void
print_time (time_t time, const char * message)
{
    // From https://community.progress.com/s/article/P129473.
    //
    setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();

    // Conversion.
    //
    struct tm * p_time_info = localtime(&time);

    char buffer[50] = {0};
    strftime(buffer, sizeof(buffer), "%c", p_time_info);

    ESP_LOGI(TAG, "message: %s: %s", message, buffer);
}

static void
on_got_time (struct timeval * p_tv)
{
    // Time since 1970.
    //
    printf("secs %ld\n", p_tv->tv_sec);

    print_time(p_tv->tv_sec, "Time at callback");

    for (int32_t idx = 0; idx < 5; ++idx)
    {
        time_t now = 0;
        time(&now);
        print_time(now, "In loop");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    example_disconnect();
}

void
app_main (void)
{
    // First time entering here the system time is not initialized.
    //
    time_t now = 0;
    time(&now);
    print_time(now, "Beginning of application");

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
    sntp_set_time_sync_notification_cb(on_got_time);
}
