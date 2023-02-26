#include <stdio.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <mdns.h>
#include "cJSON.h"
#include "connect.h"
#include "toggle_led.h"

#define TAG     "test"

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
    httpd_handle_t h_server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&h_server, &config));

    httpd_uri_t default_url = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = on_default_url
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(h_server, &default_url));

    httpd_uri_t toggle_led_url = {
        .uri = "/api/toggle-led",
        .method = HTTP_POST,
        .handler = on_toggle_led_url
    };

    ESP_ERROR_CHECK(httpd_register_uri_handler(h_server, &toggle_led_url));
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

    wifi_init();
    ESP_ERROR_CHECK(wifi_connect_sta("TP-Link_FD14", "72910153", 10000));

    start_mdns_service();
    init_server();
}
