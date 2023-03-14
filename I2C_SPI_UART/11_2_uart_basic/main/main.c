#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"

#define TXD_PIN     4
#define RXD_PIN     5

#define RX_BUF_SIZE 1024

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
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, RX_BUF_SIZE, 0, 0, NULL, 0));

    char message[] = "ping";
    printf("sending %s\n", message);

    uart_write_bytes(UART_NUM_1, message, sizeof(message));

    char incoming_message[RX_BUF_SIZE] = {0};
    uart_read_bytes(UART_NUM_1, incoming_message, RX_BUF_SIZE, pdMS_TO_TICKS(5000));

    printf("received %s\n", incoming_message);
}
