#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_err.h>
#include <stdio.h>

#define TAG     "example"
#define MAX_AP  20

static esp_event_handler_instance_t g_instance_any_id;
static esp_event_handler_instance_t g_instance_got_ip;

static void
wifi_event_handler (void * p_arg, esp_event_base_t event_base,
                    int32_t event_id, void * p_event_data)
{
    return;
}

static char *
get_auth_mode_name (wifi_auth_mode_t auth_mode)
{
    static char * names[] = {"OPEN", "WEP", "WPA PSK", "WPA2 PSK",
                             "WPA WPA2 PSK", "WPA2 ENTERPRISE",
                             "WPA3 PSK", "WPA2 WPA3 PSK", "WAPI PSK", "MAX"};
    return names[auth_mode];
}

static void
wifi_init (void)
{
    // Initialize nvs.
    //
    esp_err_t err = nvs_flash_init();

    if (ESP_OK == err)
    {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        // Too much configurations are needed, so we use WIFI_INIT_CONFIG_DEFAULT().
        //
        wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
        // Wi-Fi initialization.
        //
        ESP_ERROR_CHECK((esp_wifi_init(&wifi_config)));
#if 0
        esp_netif_t * p_wifi_sta = esp_netif_create_default_wifi_sta();
#endif
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            &wifi_event_handler,
                                                            NULL,
                                                            &g_instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_STA_GOT_IP,
                                                            &wifi_event_handler,
                                                            NULL,
                                                            &g_instance_got_ip));
        
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }
}

void
app_main (void)
{
    wifi_init();

    // I don't want so search something specific, so I inizialize all 0.
    //
    wifi_scan_config_t scan_config = {
        .ssid = 0,
        .bssid = 0,
        .channel = 0,
        .show_hidden = true
    };

    // Wait until the scan is completed.
    //
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

    wifi_ap_record_t wifi_records[MAX_AP] = {0};
    uint16_t max_records = MAX_AP;

    // It return the number of found aps.
    //
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&max_records, wifi_records));

    printf("Found %d access points:\n", max_records);
    printf("\n");
    printf("               SSID              | Channel | RSSI |   Auth Mode \n");
    printf("----------------------------------------------------------------\n");
    for (int i = 0; i < max_records; i++)
    {
        printf("%32s | %7d | %4d | %12s\n", (char *)wifi_records[i].ssid,
               wifi_records[i].primary, wifi_records[i].rssi, get_auth_mode_name(wifi_records[i].authmode));
    }
    printf("----------------------------------------------------------------\n");
}
