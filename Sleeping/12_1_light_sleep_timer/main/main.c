#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_sleep.h>
#include <time.h>
#include <sys/time.h>
#include <esp32/rom/uart.h>
#include <stdio.h>

void
app_main (void)
{
    esp_err_t ret = 0;

    ret = esp_sleep_enable_timer_wakeup(5000000);

    if (ESP_OK == ret)
    {
#if 0
        // It is printed after wake-up!!
        //
        printf("Going for a nap\n");
#elif 0
        // It is printed after wake-up!!
        //
        fprintf(stderr, "Going for a nap\n");
#elif 1
        // It is printed before wake-up!!
        //
        printf("Going for a nap\n");

        // Wait for empty UART buffer.
        //
        uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);
#endif

        int64_t before = esp_timer_get_time();

        esp_light_sleep_start();

        int64_t after = esp_timer_get_time();

        printf("Napped for %lld ms\n", (after - before) / 1000);
    }

}
