#include "pushbtn.h"
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <stdio.h>
#include <stdlib.h>
#include <driver/gpio.h>
#include <cJSON.h>
#include "main.h"

#define BTN GPIO_NUM_0

static xSemaphoreHandle gh_btn_sem = NULL;

static void IRAM_ATTR on_btn_pushed(void * p_args);
static void task_btn_push(void * p_param);

void
init_button (void)
{
    gh_btn_sem = xSemaphoreCreateBinary();
    xTaskCreate(task_btn_push, "btn_push_task", 2048, NULL, 5, NULL);

    gpio_pad_select_gpio(BTN);
    gpio_set_direction(BTN, GPIO_MODE_INPUT);
    gpio_set_intr_type(BTN, GPIO_INTR_ANYEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BTN, on_btn_pushed, NULL);
}

static void IRAM_ATTR
on_btn_pushed (void * p_args)
{
    xSemaphoreGiveFromISR(gh_btn_sem, NULL);
}

static void
task_btn_push (void * p_param)
{
    for (;;)
    {
        xSemaphoreTake(gh_btn_sem, portMAX_DELAY);

        cJSON * p_payload = cJSON_CreateObject();
        cJSON_AddBoolToObject(p_payload, "btn_state", gpio_get_level(BTN));
        char * p_message = cJSON_Print(p_payload);

        printf("message: %s\n", p_message);
        send_websocket_message(p_message);

        cJSON_Delete(p_payload);
        p_payload = NULL;
        free(p_message);
        p_message = NULL;
    }
}
