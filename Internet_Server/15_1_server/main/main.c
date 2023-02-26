#include <stdio.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include "connect.h"

#define TAG     "test"

static esp_err_t
on_default_url (httpd_req_t * p_req)
{
    ESP_LOGI(TAG, "URL: %s", p_req->uri);
    httpd_resp_sendstr(p_req, "hello world");

    return ESP_OK;
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
}

void
app_main (void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init();
    ESP_ERROR_CHECK(wifi_connect_sta("TP-Link_FD14", "72910153", 10000));
    init_server();
}
