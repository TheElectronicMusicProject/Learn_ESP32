#include "freertos/FreeRTOS.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_bt.h"
// Host controller interface
//
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
// Host stack
//
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include <stdio.h>

#define DEVICE_NAME     "MY BLE DEVICE"
#define DEVICE_SERVER   DEVICE_NAME

uint8_t g_ble_add_type = 0;

static int
ble_gap_event (struct ble_gap_event * p_event, void * p_arg)
{
    struct ble_hs_adv_fields fields = {0};

    switch (p_event->type)
    {
        case BLE_GAP_EVENT_DISC:
            ble_hs_adv_parse_fields(&fields, p_event->disc.data, p_event->disc.length_data);
            printf("discovered device %.*s\n", fields.name_len, fields.name);

            // I espect a device named as in DEVICE_SERVER.
            // If found, we try the connection.
            //
            if ((strlen(DEVICE_SERVER) == fields.name_len) &&
                (0 == memcmp(fields.name, DEVICE_SERVER, strlen(DEVICE_SERVER))))
            {
                printf("device found\n");
                ble_gap_disc_cancel();

                // If it connects, it calls ble_gap_event() again.
                //
                ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &p_event->disc.addr, 1000, NULL, ble_gap_event, NULL);

            }
        break;
        case BLE_GAP_EVENT_CONNECT:
            ESP_LOGI("GAP", "BLE_GAP_EVENT_CONNECT %s", 0 == p_event->connect.status ? "OK" : "Failed");

            // If failed, restart advertise again.
            //
            if (p_event->connect.status != 0)
            {
            }
        break;
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI("GAP", "BLE_GAP_EVENT_DISCONNECT");
        break;
        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI("GAP", "BLE_GAP_EVENT_ADV_COMPLETE");
        break;
        case BLE_GAP_EVENT_SUBSCRIBE:
            ESP_LOGI("GAP", "BLE_GAP_EVENT_SUBSCRIBE");
        break;
        default:
        break;
    }
    return 0;
}

static void
ble_app_scan (void)
{
    struct ble_gap_disc_params ble_gap_disc_params;
    ble_gap_disc_params.filter_duplicates = 1;
    ble_gap_disc_params.passive = 1;
    ble_gap_disc_params.filter_policy = 0;
    ble_gap_disc_params.itvl = 0;
    ble_gap_disc_params.limited = 0;
    ble_gap_disc_params.window = 0;

    ble_gap_disc(g_ble_add_type, BLE_HS_FOREVER, &ble_gap_disc_params, ble_gap_event, NULL);
}

static void
ble_app_on_sync (void)
{
    // Automatically creates the best address.
    //
    ble_hs_id_infer_auto(0, &g_ble_add_type);
    ble_app_scan();
}

static void
task_host (void * p_param)
{
    nimble_port_run();
}

void
app_main (void)
{
    // Necessary for Bluetooth.
    //
    ESP_ERROR_CHECK(nvs_flash_init());

    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());
    nimble_port_init();

    // Creation of generic access profile.
    //
    ble_svc_gap_device_name_set(DEVICE_NAME);
    ble_svc_gap_init();

    // Already initialized.
    //
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    
    // Nible has a particular task.
    //
    nimble_port_freertos_init(task_host);
}
