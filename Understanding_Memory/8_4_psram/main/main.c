#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <stdio.h>

/****************************************************************
 * FOR THIS EXAMPLE YOU NEED ESP32-WROVER BECAUSE IT HAS SPIRAM *
 ****************************************************************/

static void
print_memory (void)
{
    ESP_LOGI("memory", "stack %d, total ram %d, internal memory %d, external memory %d\n",
             uxTaskGetStackHighWaterMark(NULL), heap_caps_get_free_size(MALLOC_CAP_8BIT),
             heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

void
app_main (void)
{
    print_memory();
    char * buffer = heap_caps_malloc(10, MALLOC_CAP_SPIRAM);
}
