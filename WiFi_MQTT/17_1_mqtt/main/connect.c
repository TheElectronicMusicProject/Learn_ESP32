#include "connect.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"

#define TAG         "wifi"

extern TaskHandle_t     gh_task_handle;
extern const uint32_t   g_wifi_connect;

static char *
print_disconnection_error (wifi_err_reason_t reason)
{
    switch (reason)
    {
        case WIFI_REASON_UNSPECIFIED:
            return "WIFI_REASON_UNSPECIFIED";
        case WIFI_REASON_AUTH_EXPIRE:
            return "WIFI_REASON_AUTH_EXPIRE";
        case WIFI_REASON_AUTH_LEAVE:
            return "WIFI_REASON_AUTH_LEAVE";
        case WIFI_REASON_ASSOC_EXPIRE:
            return "WIFI_REASON_ASSOC_EXPIRE";
        case WIFI_REASON_ASSOC_TOOMANY:
            return "WIFI_REASON_ASSOC_TOOMANY";
        case WIFI_REASON_NOT_AUTHED:
            return "WIFI_REASON_NOT_AUTHED";
        case WIFI_REASON_NOT_ASSOCED:
            return "WIFI_REASON_NOT_ASSOCED";
        case WIFI_REASON_ASSOC_LEAVE:
            return "WIFI_REASON_ASSOC_LEAVE";
        case WIFI_REASON_ASSOC_NOT_AUTHED:
            return "WIFI_REASON_ASSOC_NOT_AUTHED";
        case WIFI_REASON_DISASSOC_PWRCAP_BAD:
            return "WIFI_REASON_DISASSOC_PWRCAP_BAD";
        case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
            return "WIFI_REASON_DISASSOC_SUPCHAN_BAD";
        case WIFI_REASON_IE_INVALID:
            return "WIFI_REASON_IE_INVALID";
        case WIFI_REASON_MIC_FAILURE:
            return "WIFI_REASON_MIC_FAILURE";
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
            return "WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT";
        case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
            return "WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT";
        case WIFI_REASON_IE_IN_4WAY_DIFFERS:
            return "WIFI_REASON_IE_IN_4WAY_DIFFERS";
        case WIFI_REASON_GROUP_CIPHER_INVALID:
            return "WIFI_REASON_GROUP_CIPHER_INVALID";
        case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
            return "WIFI_REASON_PAIRWISE_CIPHER_INVALID";
        case WIFI_REASON_AKMP_INVALID:
            return "WIFI_REASON_AKMP_INVALID";
        case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
            return "WIFI_REASON_UNSUPP_RSN_IE_VERSION";
        case WIFI_REASON_INVALID_RSN_IE_CAP:
            return "WIFI_REASON_INVALID_RSN_IE_CAP";
        case WIFI_REASON_802_1X_AUTH_FAILED:
            return "WIFI_REASON_802_1X_AUTH_FAILED";
        case WIFI_REASON_CIPHER_SUITE_REJECTED:
            return "WIFI_REASON_CIPHER_SUITE_REJECTED";
        case WIFI_REASON_INVALID_PMKID:
            return "WIFI_REASON_INVALID_PMKID";
        case WIFI_REASON_BEACON_TIMEOUT:
            return "WIFI_REASON_BEACON_TIMEOUT";
        case WIFI_REASON_NO_AP_FOUND:
            return "WIFI_REASON_NO_AP_FOUND";
        case WIFI_REASON_AUTH_FAIL:
            return "WIFI_REASON_AUTH_FAIL";
        case WIFI_REASON_ASSOC_FAIL:
            return "WIFI_REASON_ASSOC_FAIL";
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
            return "WIFI_REASON_HANDSHAKE_TIMEOUT";
        case WIFI_REASON_CONNECTION_FAIL:
            return "WIFI_REASON_CONNECTION_FAIL";
        case WIFI_REASON_AP_TSF_RESET:
            return "WIFI_REASON_AP_TSF_RESET";
        case WIFI_REASON_ROAMING:
            return "WIFI_REASON_ROAMING";
        default:
            return "OTHER ERROR";
    }

    return "";
}

static void
wifi_event_handler (void * p_arg, esp_event_base_t event_base,
                    int32_t event_id, void * p_event_data)
{
    switch (event_id)
    {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "connecting...");
            esp_wifi_connect();
        break;

        case SYSTEM_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "connected");
        break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
        {
            system_event_sta_disconnected_t * wifi_event_sta_disconnected = p_event_data;

            if (WIFI_REASON_ASSOC_LEAVE == wifi_event_sta_disconnected->reason)
            {
                ESP_LOGI(TAG, "disconnected");
            }
            else
            {
                char * err = print_disconnection_error(wifi_event_sta_disconnected->reason); 
                ESP_LOGI(TAG, "disconnected: %s", err);
                esp_wifi_connect();
            }
        }
        break;

        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "got ip");
            xTaskNotify(gh_task_handle, g_wifi_connect, eSetValueWithOverwrite);
        break;

        case WIFI_EVENT_AP_START:
            ESP_LOGI(TAG, "ap started");
        break;

        case WIFI_EVENT_AP_STOP:
            ESP_LOGI(TAG, "ap stopped");
        break;

        default:
        break;
    }
}

esp_err_t
wifi_init (void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK((esp_wifi_init(&wifi_init_config)));

    // Waiting for wifi events, any id, no arguments to the handler.
    //
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               ESP_EVENT_ANY_ID,
                                               &wifi_event_handler,
                                               NULL));
    
    // Waiting for IP events, no arguments to the handler.
    //
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                               IP_EVENT_STA_GOT_IP,
                                               &wifi_event_handler,
                                               NULL));

    esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "ASUS-PC",
            .password = "04;q649G"
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    return ESP_OK;
}

void
start_wifi (void)
{
    ESP_ERROR_CHECK(esp_wifi_start());
}

void
stop_wifi (void)
{
    ESP_ERROR_CHECK(esp_wifi_stop());
}
