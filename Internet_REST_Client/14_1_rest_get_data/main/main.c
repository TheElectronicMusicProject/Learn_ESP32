#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <stdio.h>
#include "connect.h"

#define TAG         "rest_data"
#define WIFI_SSID   "OnePlus Nord2 5G"
#define WIFI_PSWD   "v6r92qf4"

esp_err_t
on_client_data (esp_http_client_event_t * p_evt)
{
    switch (p_evt->event_id)
    {
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "Length = %d", p_evt->data_len);
            printf("%.*s\n", p_evt->data_len, (char *) p_evt->data);
        break;

        default:
        break;
    }

    return ESP_OK;
}

static void
fetch_quote (void)
{
    esp_http_client_config_t http_client_config = {
        .url = "http://quotes.rest/qod",
        .method = HTTP_METHOD_GET,
        .event_handler = on_client_data
    };

    esp_http_client_handle_t h_client = esp_http_client_init(&http_client_config);
    esp_http_client_set_header(h_client, "Content-Type", "application/json");
    esp_err_t err = esp_http_client_perform(h_client);

    if (ESP_OK == err)
    {
        ESP_LOGI(TAG, "HTTP GET status = %d, content length = %d",
                 esp_http_client_get_status_code(h_client),
                 esp_http_client_get_content_length(h_client));
    }
    else
    {
        ESP_LOGI(TAG, "HTTP GET request failed: %s",
                 esp_err_to_name(err));
    }

    esp_http_client_cleanup(h_client);
    wifi_disconnect();
}

void
app_main (void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init();
    ESP_ERROR_CHECK(wifi_connect_sta(WIFI_SSID, WIFI_PSWD, 10000));
    fetch_quote();
}
