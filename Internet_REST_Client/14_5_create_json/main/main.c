/**
 * @file    main.c
 */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_http_client.h>
#include "connect.h"
#include <stdio.h>
#include <stdlib.h>
#include <cJSON.h>

#define TAG         "rest_data"
#define WIFI_SSID   "TP-Link_FD14"
#define WIFI_PSWD   "72910153"

extern const uint8_t cert[] asm("_binary_cert_cer_start");

typedef struct
{
    uint8_t * buffer;
    int32_t buffer_idx;
} chunk_payload_t;

static char *
create_email_body (void)
{
    cJSON * p_json_payload = cJSON_CreateObject();

    cJSON * p_personalizations = cJSON_CreateArray();
    cJSON_AddItemToObject(p_json_payload, "personalizations", p_personalizations);
    
    cJSON * p_personalization_0 = cJSON_CreateObject();
    cJSON_AddItemToArray(p_personalizations, p_personalization_0);

    cJSON * p_to = cJSON_CreateArray();
    cJSON_AddItemToObject(p_personalization_0, "to", p_to);
    
    cJSON * p_to_0 = cJSON_CreateObject();
    cJSON_AddStringToObject(p_to_0, "email", "otmw.electronicmusic@gmail.com");
    cJSON_AddStringToObject(p_to_0, "name", "Filippo");
    cJSON_AddItemToArray(p_to, p_to_0);

    cJSON_AddStringToObject(p_personalization_0, "subject", "Hello, world");
    cJSON * p_content = cJSON_CreateArray();
    cJSON_AddItemToObject(p_json_payload, "content", p_content);

    cJSON * p_content_0 = cJSON_CreateObject();
    cJSON_AddStringToObject(p_content_0, "type", "text/html");
    cJSON_AddStringToObject(p_content_0, "value", "<h1>Hello from otmw</h1>");
    cJSON_AddItemToArray(p_content, p_content_0);

    cJSON * p_from = cJSON_CreateObject();
    cJSON_AddItemToObject(p_json_payload, "from", p_from);
    cJSON_AddStringToObject(p_from, "email", "otmw.electronicmusic@gmail.com");
    cJSON_AddStringToObject(p_from, "name", "Filippo");

    cJSON * p_reply_to = cJSON_CreateObject();
    cJSON_AddItemToObject(p_json_payload, "reply_to", p_reply_to);
    cJSON_AddStringToObject(p_reply_to, "email", "otmw.electronicmusic@gmail.com");
    cJSON_AddStringToObject(p_reply_to, "name", "Filippo");

    char * p_payload_body = cJSON_Print(p_json_payload);
    printf("body: %s\n", p_payload_body);

    cJSON_Delete(p_json_payload);

    return p_payload_body;
}

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
send_email (void)
{
    chunk_payload_t chunk_payload = {0};

    esp_http_client_config_t http_client_config = {
        .url = "https://api.sendgrid.com/v3/mail/send",
        .method = HTTP_METHOD_POST,
        .event_handler = on_client_data,
        .user_data = &chunk_payload,
        .cert_pem = (char *) cert
    };

    esp_http_client_handle_t h_client = esp_http_client_init(&http_client_config);
    esp_http_client_set_header(h_client, "Content-Type", "application/json");
    esp_http_client_set_header(h_client, "Authorization", "Bearer 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");
    
    char * p_payload_body = create_email_body();
    esp_http_client_set_post_field(h_client, p_payload_body, strlen(p_payload_body));
    
    esp_err_t err = esp_http_client_perform(h_client);

    if (ESP_OK == err)
    {
        ESP_LOGI(TAG, "HTTP GET status = %d",
                 esp_http_client_get_status_code(h_client));
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

    if (NULL != p_payload_body)
    {
        free(p_payload_body);
        p_payload_body = NULL;
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
    send_email();
}
