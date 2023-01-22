#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "stdint.h"

#define TAG "DICE"

static int32_t
dice_role ()
{
    int32_t random = esp_random();
    int32_t positive_number = abs(random);
    int32_t dice_number = positive_number % 6 + 1;

    return dice_number;
}

void
app_main (void)
{
    for (;;)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "random number %d", dice_role());
    }
}   /* app_main() */
