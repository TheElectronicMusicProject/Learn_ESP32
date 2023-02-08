#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <stdio.h>

#define PIN_BUTTON1     GPIO_NUM_25
#define PIN_BUTTON2     GPIO_NUM_26

// A normal variabile is resetted every time.
// Here we use a special memory to store the variable.
//
RTC_DATA_ATTR int32_t g_timers_woken_up = 0;

void
app_main (void)
{
    rtc_gpio_deinit(PIN_BUTTON1);
    rtc_gpio_deinit(PIN_BUTTON2);

    //
    // Here I can use PIN_BUTTON as a normal GPIO.
    //
    
    // To use EXT1 we need to turn on the RTC (it is turned off).
    //
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
#if 0
    // PUT ONE PIN TO GND, THE OTHER TO GPIO.
    rtc_gpio_pullup_en(PIN_BUTTON1);
    rtc_gpio_pulldown_dis(PIN_BUTTON1);
    rtc_gpio_pullup_en(PIN_BUTTON2);
    rtc_gpio_pulldown_dis(PIN_BUTTON2);

    uint64_t mask = (1ULL << PIN_BUTTON1) | (1ULL << PIN_BUTTON2);

    // mask -> multiple pins
    // mode -> wakeup mode for all pins
    // The two buttons must be pressed at the same time.
    esp_sleep_enable_ext1_wakeup(mask, ESP_EXT1_WAKEUP_ALL_LOW);
#else
    // PUT ONE PIN TO 3.3V, THE OTHER TO GPIO.
    rtc_gpio_pullup_dis(PIN_BUTTON1);
    rtc_gpio_pulldown_en(PIN_BUTTON1);
    rtc_gpio_pullup_dis(PIN_BUTTON2);
    rtc_gpio_pulldown_en(PIN_BUTTON2);

    uint64_t mask = (1ULL << PIN_BUTTON1) | (1ULL << PIN_BUTTON2);

    // mask -> multiple pins
    // mode -> wakeup mode for all pins
    // The two buttons are enabled independently.
    esp_sleep_enable_ext1_wakeup(mask, ESP_EXT1_WAKEUP_ANY_HIGH);
#endif
    fprintf(stderr, "Going to sleep, woken up %d times\n", g_timers_woken_up++);

    esp_deep_sleep_start();

    // Never reaching this point because of a system reset.
    //
    printf("Waking up\n");
}
