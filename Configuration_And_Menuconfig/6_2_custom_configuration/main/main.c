#include <stdio.h>
#include <esp_log.h>
#include <sdkconfig.h>

#define TAG "CONFIG"

void
app_main (void)
{
    ESP_LOGI(TAG, "MY_STRING is %s", CONFIG_MY_STRING);
    ESP_LOGI(TAG, "MY_INT is %d", CONFIG_MY_INT);

    bool b_my_bool = false;
#ifdef CONFIG_MY_BOOL
    b_my_bool = true;
#else
    b_my_bool = false;
#endif
    ESP_LOGI(TAG, "MY_BOOL is %s", b_my_bool ? "yes" : "no");

    int option = 0;
#ifdef CONFIG_OPTION_1
    option = 1;
#elif CONFIG_OPTION_2
    option = 2;
#elif CONFIG_OPTION_3
    option = 3;
#endif
    ESP_LOGI(TAG, "MY_OPTION is %d", option);
}
