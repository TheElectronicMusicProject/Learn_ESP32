/**
 * @file    main.c
 * 
 * @par     Description
 * You must register to rapidapi.com.
 * Search for WeatherAPI.com and go to Forecast Weather API.
 * Subscribe to this API and "Test Endpoint".
 * From Code Snippets, search for (C) Libcurl and copy the https address
 * from curl_easy_setopt. Also copy headers.
 * 
 * @attention This example will work only with ESP-WROVER.
 */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_http_client.h>
#include "connect.h"
#include <stdio.h>
#include <stdlib.h>
#include "parse_weather.h"

#define TAG         "rest_data"
#define WIFI_SSID   "TP-Link_FD14"
#define WIFI_PSWD   "72910153"

extern const uint8_t cert[] asm("_binary_amazon_cer_start");

typedef struct
{
    uint8_t * buffer;
    int32_t buffer_idx;
} chunk_payload_t;

esp_err_t
on_client_data (esp_http_client_event_t * p_evt)
{
    switch (p_evt->event_id)
    {
        case HTTP_EVENT_ON_DATA:
        {
#if 0
            ESP_LOGI(TAG, "Length = %d", p_evt->data_len);
            printf("%.*s\n", p_evt->data_len, (char *) p_evt->data);
#endif
            chunk_payload_t * payload = p_evt->user_data;
            payload->buffer = (uint8_t *) realloc(payload->buffer,
                                                  payload->buffer_idx + p_evt->data_len + 1);
            memcpy(payload->buffer + payload->buffer_idx, (char *) p_evt->data, p_evt->data_len);
            payload->buffer_idx += p_evt->data_len;
            payload->buffer[payload->buffer_idx] = '\0';
#if 0
            printf("buffer**** %s\n", payload->buffer);
#endif
        }
        break;

        default:
        break;
    }

    return ESP_OK;
}

static void
fetch_quote (void)
{
    chunk_payload_t chunk_payload = {0};

    esp_http_client_config_t http_client_config = {
        .url = "https://weatherapi-com.p.rapidapi.com/forecast.json?q=Bologna&days=3",
        .method = HTTP_METHOD_GET,
        .event_handler = on_client_data,
        .user_data = &chunk_payload,
        .cert_pem = (char *) cert
    };

    esp_http_client_handle_t h_client = esp_http_client_init(&http_client_config);
    esp_http_client_set_header(h_client, "Content-Type", "application/json");
    esp_http_client_set_header(h_client, "X-RapidAPI-Key", "b7f95babb5msh03b96830013f718p13f167jsn1ea5b7782a6e");
    esp_http_client_set_header(h_client, "X-RapidAPI-Host", "weatherapi-com.p.rapidapi.com");
    esp_err_t err = esp_http_client_perform(h_client);

    if (ESP_OK == err)
    {
        ESP_LOGI(TAG, "HTTP GET status = %d",
                 esp_http_client_get_status_code(h_client));

        parse_weather((char *) chunk_payload.buffer);
    }
    else
    {
        ESP_LOGI(TAG, "HTTP GET request failed: %s",
                 esp_err_to_name(err));
    }

    if (NULL != chunk_payload.buffer)
    {
        free(chunk_payload.buffer);
        chunk_payload.buffer = NULL;
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