#include "connect.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void
wifi_init (void)
{

}

esp_err_t
wifi_connect_sta (const char * p_ssid, const char * p_pass, int32_t timeout)
{
    return 0;
}

void
wifi_connect_ap (const char * p_ssid, const char * p_pass)
{

}

void
wifi_disconnect (void)
{

}