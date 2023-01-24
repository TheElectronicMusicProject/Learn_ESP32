#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <stdio.h>

#define DEBUG

static void
calling_func (void)
{
#ifdef DEBUG
    fprintf(stderr, "## inside func 1\n");
#endif /* DEBUG */
    char buffer[30] = {0};
#ifdef DEBUG
    fprintf(stderr, "## inside func 2\n");
#endif /* DEBUG */
    memset(buffer, 'x', 30);
    buffer[sizeof(buffer) - 1] = '\0';
#ifdef DEBUG
    fprintf(stderr, "## inside func 3\n");
#endif /* DEBUG */
    printf("buffer is %s\n", buffer);
#ifdef DEBUG
    fprintf(stderr, "## inside func 4\n");
#endif /* DEBUG */

    for (int idx = 0; idx < sizeof(buffer); ++idx)
    {
        int num1 = rand() % 5;
        int known = 100;
        printf("result is %d\n", known / num1);
    }
}   /* calling_func() */

static void
task1 (void * p_param)
{
    for (;;)
    {
#ifdef DEBUG
        fprintf(stderr, "## before func\n");
#endif /* DEBUG */
        calling_func();
#ifdef DEBUG
        fprintf(stderr, "## after func\n");
#endif /* DEBUG */
        vTaskDelete(NULL);
        break;
    }
}   /* task1() */

void
app_main (void)
{
    xTaskCreate(task1, "task1", 2048, NULL, 5, NULL);
}   /* app_main() */
