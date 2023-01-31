#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <esp_system.h>
#include <stdio.h>

static void
timer_callback (TimerHandle_t h_timer)
{
    printf("timer hit %lld ms\n", esp_timer_get_time() / 1000);
    /* Mai mettere dei ritardi qui dentro */
}

void
app_main (void)
{
    printf("app started %lld ms\n", esp_timer_get_time() / 1000);
#if 0
    TimerHandle_t h_timer = xTimerCreate("my timer", pdMS_TO_TICKS(1000), pdFALSE, NULL, timer_callback);
#else
    TimerHandle_t h_timer = xTimerCreate("my timer", pdMS_TO_TICKS(1000), pdTRUE, NULL, timer_callback);
#endif
    xTimerStart(h_timer, 0);
}
