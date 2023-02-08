#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_sleep.h>
#include <stdio.h>

// A normal variabile is resetted every time.
// Here we use a special memory to store the variable.
//
RTC_DATA_ATTR int32_t g_timers_woken_up = 0;

void
app_main (void)
{
    esp_err_t ret = 0;

    ret = esp_sleep_enable_timer_wakeup(5000000);

    if (ESP_OK == ret)
    {
        fprintf(stderr, "Going to sleep, woken up %d times\n", g_timers_woken_up++);

        esp_deep_sleep_start();

        // Never reaching this point because of a system reset.
        //
        printf("Waking up\n");
    }

}
