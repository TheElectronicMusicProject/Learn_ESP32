#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_now.h"

#define TAG     "ESP-NOW"

uint8_t g_esp1[6] = {0xEC, 0x94, 0xCB, 0x7C, 0x9B, 0xB8};
uint8_t g_esp2[6] = {0xEC, 0x94, 0xCB, 0x7B, 0x2B, 0xA4};

static char *
mac_to_str (char * p_buffer, uint8_t * p_mac)
{
    sprintf(p_buffer, "%02X%02X%02X%02X%02X%02X",
            p_mac[0], p_mac[1], p_mac[2], p_mac[3], p_mac[4], p_mac[5]);
    *(p_buffer + 13) = '\0';

    return p_buffer;
}

static void
on_sent (const uint8_t * p_mac_addr, esp_now_send_status_t status)
{
    char buffer[13] = {0};

    switch (status)
    {
        case ESP_NOW_SEND_SUCCESS:
            ESP_LOGI(TAG, "message sent to %s", mac_to_str(buffer, (uint8_t *) p_mac_addr));
        break;

        case ESP_NOW_SEND_FAIL:
            ESP_LOGE(TAG, "message sent to %s FAILED!", mac_to_str(buffer, (uint8_t *) p_mac_addr));
        break;

        default:
        break;
    }
}

static void
on_receive (const uint8_t * p_mac_addr, const uint8_t * p_data, int data_len)
{
    char buffer[13] = {0};

    ESP_LOGI(TAG, "got message from %s", mac_to_str(buffer, (uint8_t *) p_mac_addr));

    printf("message %.*s\n", data_len, p_data);
}

void
app_main (void)
{
    // We need to know the MAC address of the device to connect.
    //
    uint8_t my_mac[6] = {0};

    esp_efuse_mac_get_default(my_mac);
    char my_mac_str[13] = {0};
    ESP_LOGI(TAG, "My mac %s", mac_to_str(my_mac_str, my_mac));
    ESP_LOGI(TAG, "My mac %s", my_mac_str);

    bool b_is_current_esp1 = memcmp(my_mac, g_esp1, 6) == 0;
    // I select the other esp MAC because I need it.
    //
    uint8_t * p_peer_mac = b_is_current_esp1 != 0 ? g_esp2 : g_esp1;

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK((esp_wifi_init(&wifi_init_config)));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(on_sent));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_receive));

    // Up to 20 peers.
    //
    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, p_peer_mac, 6);
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    // 250 byte is the maximum size buffer.
    //
    char send_buffer[ESP_NOW_MAX_DATA_LEN] = {0};

    for (int32_t idx = 0; idx < 200; ++idx)
    {
        sprintf(send_buffer, "Hello from %s message %d", my_mac_str, idx);

        ESP_ERROR_CHECK(esp_now_send(NULL, (uint8_t *) send_buffer, strlen(send_buffer)));

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_ERROR_CHECK(esp_now_deinit());
    ESP_ERROR_CHECK(esp_wifi_stop());
}
