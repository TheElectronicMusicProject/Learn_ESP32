#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_now.h"

#define TAG     "ESP-NOW"

typedef enum message_type_t
{
    BROADCAST,
    SEND_MESSAGE
} message_type_t;

typedef struct payload_t
{
    char message[100];
    message_type_t message_type;
} payload_t;

typedef struct payload_ext_t
{
    payload_t payload;
    uint8_t from_mac[6];
} payload_ext_t;

QueueHandle_t gh_message_recv = NULL;

static char *
mac_to_str (char * p_buffer, uint8_t * p_mac)
{
    sprintf(p_buffer, "%02X%02X%02X%02X%02X%02X",
            p_mac[0], p_mac[1], p_mac[2], p_mac[3], p_mac[4], p_mac[5]);
    *(p_buffer + 13) = '\0';

    return p_buffer;
}

static void
brodcast_cb (TimerHandle_t h_timer)
{
    uint8_t broadcast_address[6] = {0};
    payload_t payload = {
        .message = "Ping",
        .message_type = BROADCAST
    };

    memset(broadcast_address, 0xFF, sizeof(broadcast_address));

    ESP_ERROR_CHECK(esp_now_send(broadcast_address, (uint8_t *) &payload, sizeof(payload_t)));
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

// This is an high priority task, so we'll use a queue.
//
static void
on_receive (const uint8_t * p_mac_addr, const uint8_t * p_data, int data_len)
{
    // To avoid memory corruption.
    //
    if (data_len != sizeof(payload_t))
    {
        ESP_LOGW(TAG, "Received incorrect paylaod");
        return;
    }

    payload_ext_t payload_ext = {0};
    payload_t * p_payload = (payload_t *) p_data;
    payload_ext.payload = *p_payload;
    memcpy(payload_ext.from_mac, p_mac_addr, sizeof(payload_ext.from_mac));
    xQueueSend(gh_message_recv, &payload_ext, 0);
}

static void
task_msg_received (void * p_param)
{
    char buffer[13] = {0};

    gh_message_recv = xQueueCreate(10, sizeof(payload_ext_t));

    for (;;)
    {
        payload_ext_t payload_ext = {0};
        xQueueReceive(gh_message_recv, &payload_ext, portMAX_DELAY);

        switch (payload_ext.payload.message_type)
        {
            case BROADCAST:
                if (false == esp_now_is_peer_exist(payload_ext.from_mac))
                {
                    ESP_LOGI(TAG, "got broadcast from %s", mac_to_str(buffer, (uint8_t *) payload_ext.from_mac));
                    esp_now_peer_info_t peer = {0};

                    // Broadcast to anyone.
                    //
                    memcpy(peer.peer_addr, payload_ext.from_mac, 6);
                    ESP_ERROR_CHECK(esp_now_add_peer(&peer));
                }
            break;

            case SEND_MESSAGE:
            break;

            default:
            break;
        }
    }
}

void
app_main (void)
{
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

    // Broadcast to anyone.
    //
    memset(peer.peer_addr, 0xFF, 6);
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    TimerHandle_t h_timer = xTimerCreate("broadcast", pdMS_TO_TICKS(10000), pdTRUE, NULL, brodcast_cb);
    xTimerStart(h_timer, 0);

    xTaskCreate(task_msg_received, "message received", 2 * 1024, NULL, 5, NULL);

#if 0
    // 250 byte is the maximum size buffer.
    //
    char send_buffer[ESP_NOW_MAX_DATA_LEN] = {0};

    ESP_ERROR_CHECK(esp_now_deinit());
    ESP_ERROR_CHECK(esp_wifi_stop());
#endif
}
