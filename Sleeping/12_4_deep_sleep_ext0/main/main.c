#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <stdio.h>

#define PIN_BUTTON  GPIO_NUM_25

// A normal variabile is resetted every time.
// Here we use a special memory to store the variable.
//
RTC_DATA_ATTR int32_t g_timers_woken_up = 0;

void
app_main (void)
{
    rtc_gpio_deinit(PIN_BUTTON);

    //
    // Here I can use PIN_BUTTON as a normal GPIO.
    //

    rtc_gpio_pullup_en(PIN_BUTTON);
    rtc_gpio_pulldown_dis(PIN_BUTTON);

    esp_sleep_enable_ext0_wakeup(PIN_BUTTON, 0);

    fprintf(stderr, "Going to sleep, woken up %d times\n", g_timers_woken_up++);

    esp_deep_sleep_start();

    // Never reaching this point because of a system reset.
    //
    printf("Waking up\n");
}
