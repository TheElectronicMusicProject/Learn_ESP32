#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <stdio.h>

#define GPIO_SWITCH     GPIO_NUM_15

QueueHandle_t gh_interrupt_queue = NULL;

// Using DRAM for interrupts.
//
static void IRAM_ATTR
isr_gpio_handler (void * p_args)
{
    int32_t pin_number = (int32_t) p_args;

    xQueueSendFromISR(gh_interrupt_queue, &pin_number, NULL);
}

static void
task_button_pushed (void * p_param)
{
    int32_t pin_number = 0;
    int32_t count = 0;

    for (;;)
    {
        if (xQueueReceive(gh_interrupt_queue, &pin_number, portMAX_DELAY))
        {
            // Disable the interrupt (wait 20/50ms while the button has bouncing)
            gpio_isr_handler_remove(pin_number);

            // Wait the button release
            do
            {
                vTaskDelay(100 / portTICK_PERIOD_MS);
            } while (1 == gpio_get_level(pin_number));

            // Do some work
            printf("GPIO %d was pressed %d times. The state is %d\n", pin_number, count++, gpio_get_level(pin_number));
        
            // Enable the interrupt
            gpio_isr_handler_add(pin_number, isr_gpio_handler, (void *) GPIO_SWITCH);
        }
    }
}

void
app_main (void)
{
    gpio_config_t config;
    config.mode = GPIO_MODE_INPUT;
    config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.intr_type = GPIO_INTR_POSEDGE;
    // 64 bit section of memory
    // for more pins: (1ULL << GPIO_SWITCH) | (1ULL << 12) | (1ULL << 1) | ...
    config.pin_bit_mask = (1ULL << GPIO_SWITCH);

    gpio_config(&config);

    gh_interrupt_queue = xQueueCreate(10, sizeof(int32_t));
    BaseType_t ret = xTaskCreate(task_button_pushed, "button pushed task", 2048, NULL, 1, NULL);
    assert(pdPASS == ret);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_SWITCH, isr_gpio_handler, (void *) GPIO_SWITCH);
}
