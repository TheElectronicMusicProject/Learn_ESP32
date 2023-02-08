#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_sleep.h>
#include <time.h>
#include <sys/time.h>
#include <esp32/rom/uart.h>
// I can use pins while sleeping
#include <driver/rtc_io.h>
#include <stdio.h>

#define INPUT_PIN       GPIO_NUM_0

void
app_main (void)
{
    esp_err_t ret = 0;

    gpio_pad_select_gpio(INPUT_PIN);
    gpio_set_direction(INPUT_PIN, GPIO_MODE_INPUT);
    gpio_wakeup_enable(INPUT_PIN, GPIO_INTR_LOW_LEVEL);

    esp_sleep_enable_gpio_wakeup();

    ret = esp_sleep_enable_timer_wakeup(5000000);

    for (;;)
    {
        if (ESP_OK == ret)
        {
            // Wait for button release.
            //
            if (0 == rtc_gpio_get_level(INPUT_PIN))
            {
                do
                {
                    vTaskDelay(pdMS_TO_TICKS(10));
                } while (0 == rtc_gpio_get_level(INPUT_PIN));
            }

            // It is printed before wake-up!!
            //
            printf("Going for a nap\n");

            // Wait for empty UART buffer.
            //
            uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);

            int64_t before = esp_timer_get_time();

            esp_light_sleep_start();

            int64_t after = esp_timer_get_time();

            // Which reason wakes-up the CPU?
            //
            esp_sleep_source_t reason = esp_sleep_get_wakeup_cause();

            printf("Napped for %lld ms, reason was %s\n",
                   (after - before) / 1000,
                   reason == ESP_SLEEP_WAKEUP_TIMER ? "timer" : "button");
        }
    }
}
