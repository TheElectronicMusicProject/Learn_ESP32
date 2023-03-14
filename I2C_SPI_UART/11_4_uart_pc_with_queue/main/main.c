#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"

#define TXD_PIN     21
#define RXD_PIN     19

#define RX_BUF_SIZE 1024
#define TX_BUF_SIZE 1024
#define PATTERN_LEN 3
#define QUEUE_SIZE  20
#define TAG         "uart"

QueueHandle_t gh_queue_uart = NULL;

static void
task_event (void * p_param)
{
    uart_event_t uart_event = {0};
    uint8_t * received_buffer = (uint8_t *) malloc(RX_BUF_SIZE);

    for (;;)
    {
        if (xQueueReceive(gh_queue_uart, &uart_event, portMAX_DELAY))
        {
            switch (uart_event.type)
            {
                case UART_DATA:
                    ESP_LOGI(TAG, "UART_DATA");
                    uart_read_bytes(UART_NUM_1, received_buffer, uart_event.size, portMAX_DELAY);
                    printf("received: %.*s\n", uart_event.size, received_buffer);
                break;

                case UART_BREAK:
                    ESP_LOGI(TAG, "UART_BREAK");
                break;

                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "UART_BUFFER_FULL");
                break;

                // In case of overflow, release all the data in the queue.
                //
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "UART_FIFO_OVF");
                    uart_flush_input(UART_NUM_1);
                    xQueueReset(gh_queue_uart);
                break;

                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "UART_FRAME_ERR");
                break;

                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "UART_PARITY_ERR");
                break;

                case UART_DATA_BREAK:
                    ESP_LOGI(TAG, "UART_DATA_BREAK");
                break;

                // The pattern "+++" must be at the end of the string.
                // e.g. -> "hello world+++"
                //
                case UART_PATTERN_DET:
                {
                    ESP_LOGI(TAG, "UART_PATTERN_DET");
                    uint32_t data_length = 0;
                    uart_get_buffered_data_len(UART_NUM_1, &data_length);
                    int32_t pos = uart_pattern_pop_pos(UART_NUM_1);
                    ESP_LOGI(TAG, "Detected %d pos %d\n", data_length, pos);

                    // I extract the packet from the patter (so I substract 3).
                    //
                    uart_read_bytes(UART_NUM_1, received_buffer, data_length - PATTERN_LEN, pdMS_TO_TICKS(100));
                    uint8_t pat[PATTERN_LEN + 1] = {0};
                    uart_read_bytes(UART_NUM_1, pat, PATTERN_LEN, pdMS_TO_TICKS(100));
                    printf("data: %.*s | pattern %s\n", data_length - PATTERN_LEN, received_buffer, pat);
                }
                break;

                case UART_EVENT_MAX:
                    /* Fall through */
                default:
                break;
            }
        }
    }
}

void
app_main (void)
{
    uart_config_t uart_conf = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    // UART_NUM_0 is tipically used for logging.
    //
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_conf));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // I won't use a buffer in this case, neither queue.
    //
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, RX_BUF_SIZE, TX_BUF_SIZE, QUEUE_SIZE, &gh_queue_uart, 0));

    // Recognize a pattern from the buffer.
    // We are especting "+++".
    // We use quick timeouts.
    //
    ESP_ERROR_CHECK(uart_enable_pattern_det_intr(UART_NUM_1, '+', PATTERN_LEN, 10000, 10, 10));
    ESP_ERROR_CHECK(uart_pattern_queue_reset(UART_NUM_1, QUEUE_SIZE));

    xTaskCreate(task_event, "uart_event_task", 2048, NULL, 10, NULL);
}
