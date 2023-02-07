#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/ledc.h>
#include <esp_err.h>
#include <stdio.h>

void
app_main (void)
{
    // 1) Configuring the timer.
    //
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&timer);

    // 2) Configuring the channel (related to the timer).
    //
    ledc_channel_config_t channel = {
        .gpio_num = GPIO_NUM_4,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 512, // max value depends on duty_resolution (10 bits -> 1023)
        .hpoint = 0,
    };

    ledc_channel_config(&channel);

    for (int32_t idx = 0; idx < 1024; ++idx)
    {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, idx);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    // To avoid errors, ledc_set_duty_and_update() needs this (also it enables interrupts).
    //
    ledc_fade_func_install(0);

    for (int32_t idx = 1023; idx >= 0; --idx)
    {
        ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, idx, 0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    // Period of timer to span the duty cycle range.
    //
    for (;;)
    {
        ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1024, 1000, LEDC_FADE_WAIT_DONE);
        ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 1000, LEDC_FADE_WAIT_DONE);
    }
}
