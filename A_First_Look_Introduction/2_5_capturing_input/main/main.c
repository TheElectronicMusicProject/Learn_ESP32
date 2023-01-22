#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

void
app_main (void)
{
    char chr = 0;
    char str[100];

    memset(str, 0, sizeof(str));

    while (chr != '\n')
    {
        chr = getchar();

        if (chr != 0xFF)
        {
            str[strlen(str)] = chr;
            printf("%c", chr);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    printf("You typed %s\n", str);
}   /* app_main() */
