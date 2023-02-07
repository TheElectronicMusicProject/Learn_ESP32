#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <stdio.h>

#define GPIO_SWITCH     GPIO_NUM_15
#define GPIO_LED        GPIO_NUM_2

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
            printf("GPIO %d was pressed %d times. The state is %d\n", pin_number, count++, gpio_get_level(GPIO_SWITCH));
        }
    }
}

void
app_main (void)
{
    gpio_pad_select_gpio(GPIO_LED);
    gpio_set_direction(GPIO_LED, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(GPIO_SWITCH);
    gpio_set_direction(GPIO_SWITCH, GPIO_MODE_INPUT);
    gpio_pulldown_en(GPIO_SWITCH);
    gpio_pullup_dis(GPIO_SWITCH);
    gpio_set_intr_type(GPIO_SWITCH, GPIO_INTR_POSEDGE);

    gh_interrupt_queue = xQueueCreate(10, sizeof(int32_t));
    BaseType_t ret = xTaskCreate(task_button_pushed, "button pushed task", 2048, NULL, 1, NULL);
    assert(pdPASS == ret);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_SWITCH, isr_gpio_handler, (void *) GPIO_SWITCH);
}
