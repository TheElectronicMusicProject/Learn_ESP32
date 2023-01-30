#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <esp_timer.h>
#include <driver/gpio.h>

#define TEST_2  1

static void
timer_callback (void * p_arg)
{
#if TEST_1
    printf("fired after 20 micro secs\n");
#elif TEST_2
    static bool on;

    on = !on;
    gpio_set_level(GPIO_NUM_4, on);
#endif
}

void
app_main (void)
{
#if TEST_2
    gpio_pad_select_gpio(GPIO_NUM_4);
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
#endif

    const esp_timer_create_args_t esp_timer_create_args = {
        .callback = timer_callback,
        .name = "My timer",
    };

    esp_timer_handle_t h_timer = NULL;

    esp_timer_create(&esp_timer_create_args, &h_timer);
#if TEST_1
    esp_timer_start_once(h_timer, 20);
#elif TEST_2
    esp_timer_start_periodic(h_timer, 50);
#endif

    int val = 0;

    while (true)
    {
        esp_timer_dump(stdout);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        if (5 == val++)
        {
            // Cancello il timer dopo 5 secondi.
            //
            esp_timer_stop(h_timer);
            esp_timer_delete(h_timer);
        }
    }
}
