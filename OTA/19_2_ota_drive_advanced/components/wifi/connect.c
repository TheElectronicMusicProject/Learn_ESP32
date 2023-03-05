#include "connect.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"

#define TAG "wifi"

static esp_netif_t * gp_wifi_sta = NULL;
static esp_netif_t * gp_wifi_ap = NULL;
static EventGroupHandle_t g_wifi_event;
static const int32_t CONNECTED_GOT_IP = (1 << 0);
static const int32_t DISCONNECTED = (1 << 1);

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
                xEventGroupSetBits(g_wifi_event, DISCONNECTED);
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
            xEventGroupSetBits(g_wifi_event, CONNECTED_GOT_IP);
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

void
wifi_init (void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK((esp_wifi_init(&wifi_config)));

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

    esp_wifi_set_storage(WIFI_STORAGE_RAM);
}

esp_err_t
wifi_connect_sta (const char * p_ssid, const char * p_pass, int32_t timeout)
{
    g_wifi_event = xEventGroupCreate();
    wifi_config_t wifi_config = {0};

    gp_wifi_sta = esp_netif_create_default_wifi_sta();

    strncpy((char *) wifi_config.sta.ssid, p_ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *) wifi_config.sta.password, p_pass, sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t result = xEventGroupWaitBits(g_wifi_event, CONNECTED_GOT_IP | DISCONNECTED,
                                             pdTRUE, pdFALSE, pdMS_TO_TICKS(timeout));

    if (CONNECTED_GOT_IP == result)
    {
        return ESP_OK;
    }

    return ESP_FAIL;
}

void
wifi_connect_ap (const char * p_ssid, const char * p_pass)
{
    wifi_config_t wifi_config = {0};

    gp_wifi_ap = esp_netif_create_default_wifi_ap();

    strncpy((char *) wifi_config.ap.ssid, p_ssid, sizeof(wifi_config.ap.ssid) - 1);
    strncpy((char *) wifi_config.ap.password, p_pass, sizeof(wifi_config.ap.password) - 1);
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifi_config.ap.max_connection = 4;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void
wifi_disconnect (void)
{
    esp_wifi_disconnect();
    esp_wifi_stop();
}