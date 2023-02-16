#include <stdio.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"  // for wifi credentials
#include "esp_http_client.h"
// We use a particular component used to use a quick connection for
// demonstrations.
//
#include "protocol_examples_common.h"

static esp_err_t
client_event (esp_http_client_event_t * p_evt)
{
    switch (p_evt->event_id)
    {
        // Data receive.
        //
        case HTTP_EVENT_ON_DATA:
            // %s for string, .*s requires also length.
            //
            printf("%.*s\n", p_evt->data_len, (char *) p_evt->data);
        break;

        default:
        break;
    }

    return ESP_OK;
}

void
app_main (void)
{
    nvs_flash_init();

    esp_netif_init();
    esp_event_loop_create_default();
    example_connect();

    esp_http_client_config_t client_config = {
        .url = "http://google.com",
        .event_handler = client_event,
    };

    esp_http_client_handle_t client = esp_http_client_init(&client_config);
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}
