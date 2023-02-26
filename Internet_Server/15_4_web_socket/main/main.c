#include "main.h"
#include <stdio.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <mdns.h>
#include "cJSON.h"
#include "connect.h"
#include "toggle_led.h"
#include "pushbtn.h"

#define TAG             "test"
#define WS_MAX_SIZE     1024

static httpd_handle_t gh_server = NULL;
static int32_t g_client_session_id = 0;

esp_err_t
send_websocket_message (char * p_message)
{
    if (NULL == p_message)
    {
        ESP_LOGE(TAG, "no client_session_id");
        return -1;
    }

    httpd_ws_frame_t ws_message = {
        .final = true,
        .fragmented = false,
        .len = strlen(p_message),
        .payload = (uint8_t *) p_message,
        .type = HTTPD_WS_TYPE_TEXT
    };

    return httpd_ws_send_frame_async(gh_server, g_client_session_id, &ws_message);
}

static esp_err_t
on_websocket_url (httpd_req_t * p_req)
{
    g_client_session_id = httpd_req_to_sockfd(p_req);

    if (HTTP_GET == p_req->method)
    {
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(ws_pkt));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = (uint8_t *) malloc(WS_MAX_SIZE * sizeof(uint8_t));
    httpd_ws_recv_frame(p_req, &ws_pkt, WS_MAX_SIZE);
    
    printf("ws payload: %.*s\n", ws_pkt.len, ws_pkt.payload);

    free(ws_pkt.payload);
    ws_pkt.payload = NULL;

    char * p_response = "connected OK :-)";
    httpd_ws_frame_t ws_response = 
    {
        .final = true,
        .fragmented = false,
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t *) p_response,
        .len = strlen(p_response)
    };

    return httpd_ws_send_frame(p_req, &ws_response);
}

static esp_err_t
on_default_url (httpd_req_t * p_req)
{
    ESP_LOGI(TAG, "URL: %s", p_req->uri);
    httpd_resp_sendstr(p_req, "hello world");

    return ESP_OK;
}

static esp_err_t
on_toggle_led_url (httpd_req_t * p_req)
{
    char buffer[100];
    memset(buffer, 0, sizeof(buffer));
    httpd_req_recv(p_req, buffer, p_req->content_len);

    cJSON * p_payload = cJSON_Parse(buffer);

    if (NULL != p_payload)
    {
        cJSON * p_is_on_json = cJSON_GetObjectItem(p_payload, "is_on");
        bool b_is_on = cJSON_IsTrue(p_is_on_json);

        cJSON_Delete(p_payload);
        p_payload = NULL;

        toggle_led(b_is_on);

        httpd_resp_set_status(p_req, "204 NO CONTENT");
        httpd_resp_send(p_req, NULL, 0);

        return ESP_OK;
    }

    return ESP_FAIL;
}

static void
init_server (void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&gh_server, &config));

    httpd_uri_t default_url = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = on_default_url
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(gh_server, &default_url));

    httpd_uri_t toggle_led_url = {
        .uri = "/api/toggle-led",
        .method = HTTP_POST,
        .handler = on_toggle_led_url
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(gh_server, &toggle_led_url));

    httpd_uri_t websocket_url = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = on_websocket_url,
        .is_websocket = true
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(gh_server, &websocket_url));
}

static void
start_mdns_service (void)
{
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("my-esp32"));
    ESP_ERROR_CHECK(mdns_instance_name_set("Learn esp32 thing"));

    if (false == mdns_hostname_exists("my-esp32"))
    {
        ESP_LOGE(TAG, "Error on hostname");
    }
}

void
app_main (void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    init_led();
    init_button();

    wifi_init();
    ESP_ERROR_CHECK(wifi_connect_sta("TP-Link_FD14", "72910153", 10000));

    start_mdns_service();
    init_server();
}
