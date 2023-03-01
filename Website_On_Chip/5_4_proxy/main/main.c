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
#include "esp_spiffs.h"
#include "esp_wifi.h"

#define TAG             "test"
#define WS_MAX_SIZE     1024
#define MAX_APS         20

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

    esp_vfs_spiffs_conf_t esp_vfs_spiffs_conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_vfs_spiffs_register(&esp_vfs_spiffs_conf);

    char path[600] = {0};
    
    if (0 == strcmp(p_req->uri, "/"))
    {
        strcpy(path, "/spiffs/index.html");
    }
    else
    {
        sprintf(path, "/spiffs%s", p_req->uri);
    }

    char * p_ext = strrchr(path, '.');

    if (0 == strcmp(p_ext, ".css"))
    {
        httpd_resp_set_type(p_req, "text/css");
    }

    if (0 == strcmp(p_ext, ".js"))
    {
        httpd_resp_set_type(p_req, "text/javascript");
    }

    if (0 == strcmp(p_ext, ".png"))
    {
        httpd_resp_set_type(p_req, "image/png");
    }

    FILE * ph_file = fopen(path, "r");

    if (NULL == ph_file)
    {
        httpd_resp_send_404(p_req);
        esp_vfs_spiffs_unregister(NULL);
        return ESP_OK;
    }

    char line_read[256] = {0};

    while (fgets(line_read, sizeof(line_read), ph_file))
    {
        httpd_resp_sendstr_chunk(p_req, line_read);
    }

    httpd_resp_sendstr_chunk(p_req, NULL);
    esp_vfs_spiffs_unregister(NULL);

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

static esp_err_t
on_get_ap_url (httpd_req_t * p_req)
{
    esp_wifi_set_mode(WIFI_MODE_APSTA);

    wifi_scan_config_t wifi_scan_config = {
        .bssid = 0,
        .ssid = 0,
        .channel = 0,
        .show_hidden = true
    };

    esp_wifi_scan_start(&wifi_scan_config, true);

    wifi_ap_record_t wifi_ap_record[MAX_APS] = {0};
    uint16_t ap_count = MAX_APS;

    esp_wifi_scan_get_ap_records(&ap_count, wifi_ap_record);

    cJSON * p_wifi_scan_json = cJSON_CreateArray();

    for (uint32_t idx = 0; idx < ap_count; ++idx)
    {
        cJSON * p_element = cJSON_CreateObject();
        cJSON_AddStringToObject(p_element, "ssid", (char *) wifi_ap_record[idx].ssid);
        cJSON_AddNumberToObject(p_element, "rssi", wifi_ap_record[idx].rssi);
        cJSON_AddItemToArray(p_wifi_scan_json, p_element);
    }

    char * p_json_str = cJSON_Print(p_wifi_scan_json);

    httpd_resp_set_type(p_req, "application/json");
    httpd_resp_sendstr(p_req, p_json_str);

    cJSON_Delete(p_wifi_scan_json);
    p_wifi_scan_json = NULL;
    free(p_json_str);
    p_json_str = NULL;

    return ESP_OK;
}

static void
init_server (void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_ERROR_CHECK(httpd_start(&gh_server, &config));

    httpd_uri_t get_ap_url = {
        .uri = "/api/get-ap-list",
        .method = HTTP_GET,
        .handler = on_get_ap_url
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(gh_server, &get_ap_url));

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

    // I put this at the and because it is sequential. This matches all!
    //
    httpd_uri_t default_url = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = on_default_url
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(gh_server, &default_url));
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
    ESP_ERROR_CHECK(wifi_connect_sta("OnePlus Nord2 5G", "v6r92qf4", 10000));

    start_mdns_service();
    init_server();
}
