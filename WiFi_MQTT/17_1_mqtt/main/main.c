#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <cJSON.h>
#include <stdio.h>
#include <mqtt_client.h>
#include "connect.h"

#define TAG     "MQTT"

TaskHandle_t        gh_task_handle = NULL;
QueueHandle_t       gh_reading_queue = NULL;
const uint32_t      g_wifi_connect = (1 << 1);
const uint32_t      g_mqtt_connect = (1 << 2);
const uint32_t      g_mqtt_publish = (1 << 3);

static void
mqtt_event_handler_cb (esp_mqtt_event_handle_t event)
{
    switch (event->event_id)
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            xTaskNotify(gh_task_handle, g_mqtt_connect, eSetValueWithOverwrite);
        break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            xTaskNotify(gh_task_handle, g_mqtt_publish, eSetValueWithOverwrite);
        break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void
handler_mqtt_event (void * p_event_handler_arg,
                    esp_event_base_t event_base,
                    int32_t event_id,
                    void * p_event_data)
{
    mqtt_event_handler_cb(p_event_data);
}

static void
mqtt_logic (int32_t reading)
{
    uint32_t command = 0;
    esp_mqtt_client_config_t mqtt_config = {
        .uri = "mqtt://test.mosquitto.org:1883"
    };

    esp_mqtt_client_handle_t h_client = NULL;
    ESP_LOGW(TAG, "in mqtt_logic");

    for (;;)
    {
        xTaskNotifyWait(0, 0, &command, portMAX_DELAY);

        switch (command)
        {
            case g_wifi_connect:
                ESP_LOGW(TAG, "in g_wifi_connect");
                h_client = esp_mqtt_client_init(&mqtt_config);
                esp_mqtt_client_register_event(h_client, ESP_EVENT_ANY_ID, handler_mqtt_event, h_client);
                esp_mqtt_client_start(h_client);
            break;

            case g_mqtt_connect:
                ESP_LOGW(TAG, "in g_mqtt_connect");
                // Subscribe
                //
                esp_mqtt_client_subscribe(h_client, "/topic/my/subscription/1", 2);

                // Publish
                //
                char data[50] = {0};
                sprintf(data, "%d", reading);
                printf("sending data: %d\n", reading);
                esp_mqtt_client_publish(h_client, "/topic/my/publication/1", data, strlen(data), 2, false);
            break;

            case g_mqtt_publish:
                esp_mqtt_client_stop(h_client);
                esp_mqtt_client_destroy(h_client);
                stop_wifi();
                return;
            break;

            default:
            break;
        }
    }
}

static void
on_connected (void * p_param)
{
    for (;;)
    {
        int32_t sensor_reading = 0;

        if (xQueueReceive(gh_reading_queue, (void *) &sensor_reading, portMAX_DELAY))
        {
            ESP_LOGW(TAG, "in on_connected");
            start_wifi();
            mqtt_logic(sensor_reading);
        }
    }
}

void
generate_reading (void * p_params)
{
    for (;;)
    {
        int32_t random = esp_random() % 100;

        xQueueSend(gh_reading_queue, &random, 2000 / portTICK_PERIOD_MS);
        vTaskDelay(pdMS_TO_TICKS(15000));
    }
}

void
app_main (void)
{
    gh_reading_queue = xQueueCreate(sizeof(int32_t), 10);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(wifi_init());
    xTaskCreate(&on_connected, "handle comms", 1024 * 5, NULL, 5, &gh_task_handle);
    xTaskCreate(&generate_reading, "handle readings", 1024 * 5, NULL, 5, NULL);
}
