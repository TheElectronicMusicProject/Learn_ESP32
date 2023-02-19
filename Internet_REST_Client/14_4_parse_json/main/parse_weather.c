#include "parse_weather.h"
#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <cJSON.h>
#include <esp_heap_caps.h>

#define TAG     "parse_weather"

static void *
json_malloc (size_t size)
{
    return heap_caps_malloc(size, MALLOC_CAP_8BIT);
}

static void
json_free (void * p_mem)
{
    heap_caps_free(p_mem);
}

esp_err_t
parse_weather (char * p_weather_string)
{
    // Probabily we won't have enough space to parse this json.
    //
    int32_t dram = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    printf("dram = %d\n", dram);
    
    // To successfully parse the JSON, we need the external SPI flash memory.
    // This memory can be only found in an ESP32-WROVER.
    //
    cJSON_Hooks hooks = {
        .malloc_fn = json_malloc,
        .free_fn = json_free
    };

    cJSON_InitHooks(&hooks);

    cJSON * p_weather_json = cJSON_Parse(p_weather_string);

    if (NULL == p_weather_json)
    {
        const char * err = cJSON_GetErrorPtr();

        if (NULL != err)
        {
            ESP_LOGE(TAG, "Error parsing json before %s", err);
            return ESP_FAIL;
        }
    }
    else
    {
        cJSON * p_location = cJSON_GetObjectItemCaseSensitive(p_weather_json, "location");
        cJSON * p_name = cJSON_GetObjectItemCaseSensitive(p_location, "name");

        printf("name %s\n", p_name->valuestring);
    }

    return ESP_OK;
}