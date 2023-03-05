#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include <string.h>
#include <stdio.h>
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "connect.h"

#define TAG     "OTA"

extern const uint8_t g_server_cert_pem_start[] asm("_binary_google_pem_start");
SemaphoreHandle_t gh_ota_semaphore = NULL;

static esp_err_t
client_event_handler (esp_http_client_event_t * p_evt)
{
    switch (p_evt->event_id)
    {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
        break;

        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;

        case HTTP_EVENT_HEADERS_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADERS_SENT");
        break;

        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", p_evt->header_key, p_evt->header_value);
        break;

        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", p_evt->data_len);
        break;

        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        break;

        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }

    return ESP_OK;
}

static esp_err_t
validate_image_header (esp_app_desc_t * p_incoming_ota_desc)
{
    const esp_partition_t * p_running_partition = esp_ota_get_running_partition();
    esp_app_desc_t running_partition_desc = {0};
    ESP_ERROR_CHECK(esp_ota_get_partition_description(p_running_partition, &running_partition_desc));

    ESP_LOGI(TAG, "current version is %s\n", running_partition_desc.version);
    ESP_LOGI(TAG, "new version is %s\n", p_incoming_ota_desc->version);

    if (0 == strcmp(running_partition_desc.version, p_incoming_ota_desc->version))
    {
        ESP_LOGW(TAG, "NEW VERSION IS THE SAME AS CURRENT VERSION. ABORTING");
        return ESP_FAIL;
    }

    return ESP_OK;
}

static void
task_ota (void * p_param)
{
    ESP_ERROR_CHECK(wifi_connect_sta("TP-Link_FD14", "72910153", 10000));

    for (;;)
    {
        xSemaphoreTake(gh_ota_semaphore, portMAX_DELAY);
        ESP_LOGI(TAG, "Invoking OTA");

        esp_http_client_config_t client_config = {
            .url = "https://onedrive.live.com/download?cid=AD4B4F8E4960D831&resid=AD4B4F8E4960D831%216148&authkey=ALf-LArwZmULwPU",
            .event_handler = client_event_handler,
            .transport_type = HTTP_TRANSPORT_OVER_SSL,
            //.cert_pem = (char *) g_server_cert_pem_start,
            .crt_bundle_attach = esp_crt_bundle_attach,
            .keep_alive_enable = true
        };

        esp_https_ota_config_t ota_config = {
            .http_config = &client_config
        };

        // We need the handler for OTA.
        //
        esp_https_ota_handle_t h_ota = NULL;

        esp_err_t ret = esp_https_ota_begin(&ota_config, &h_ota);

        if (ESP_OK != ret)
        {
            ESP_LOGE(TAG, "esp_https_ota_begin failed, error %s", esp_err_to_name(ret));
            wifi_disconnect();
            continue;
        }
        
        esp_app_desc_t incoming_ota_desc = {0};

        ret = esp_https_ota_get_img_desc(h_ota, &incoming_ota_desc);

        if (ESP_OK != ret)
        {
            ESP_LOGE(TAG, "esp_https_ota_get_img_desc failed, error %s", esp_err_to_name(ret));
            ret = esp_https_ota_finish(h_ota);
            wifi_disconnect();
            continue;
        }

        ret = validate_image_header(&incoming_ota_desc);

        if (ESP_OK != ret)
        {
            ESP_LOGE(TAG, "validate_image_header failed, error %s", esp_err_to_name(ret));
            ret = esp_https_ota_finish(h_ota);
            wifi_disconnect();
            continue;
        }

        // Download in progress.
        //
        while (true)
        {
            ret = esp_https_ota_perform(h_ota);

            if (ESP_ERR_HTTPS_OTA_IN_PROGRESS != ret)
            {
                break;
            }
        }

        ret = esp_https_ota_finish(h_ota);

        if (ESP_OK == ret)
        {
            printf("restarting in 5 seconds\n");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            esp_restart();
        }
        else
        {
            ESP_LOGE(TAG, "Failed to update the firmare, error %s", esp_err_to_name(ret));
        }
    }
}

static void
on_button_pushed (void * p_param)
{
    xSemaphoreGiveFromISR(gh_ota_semaphore, pdFALSE);
}

void
app_main (void)
{
    const esp_partition_t * p_running_partition = esp_ota_get_running_partition();
    esp_app_desc_t running_partition_desc = {0};
    ESP_ERROR_CHECK(esp_ota_get_partition_description(p_running_partition, &running_partition_desc));

    // To use this element, you need Git. It is related to the commit SHA.
    //
    printf("HEY!!!!!!!!!! current firmware version is: %s\n", running_partition_desc.version);

    gh_ota_semaphore = xSemaphoreCreateBinary();
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init();

    gpio_config_t gpio_conf = {
        .pin_bit_mask = 1ULL << GPIO_NUM_0,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };

    ESP_ERROR_CHECK(gpio_config(&gpio_conf));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_NUM_0, on_button_pushed, NULL));

    xTaskCreate(task_ota, "run ota", 8 * 1024, NULL, 2, NULL);
}
