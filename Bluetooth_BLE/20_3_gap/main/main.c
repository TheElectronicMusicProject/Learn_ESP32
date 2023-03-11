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

uint8_t g_ble_add_type = 0;

static void ble_app_advertise(void);

static int
ble_gap_event (struct ble_gap_event * p_event, void * p_arg)
{
    switch (p_event->type)
    {
        case BLE_GAP_EVENT_CONNECT:
            ESP_LOGI("GAP", "BLE_GAP_EVENT_CONNECT %s", 0 == p_event->connect.status ? "OK" : "Failed");

            // If failed, restart advertise again.
            //
            if (p_event->connect.status != 0)
            {
                ble_app_advertise();
            }
        break;
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI("GAP", "BLE_GAP_EVENT_DISCONNECT");
            ble_app_advertise();
        break;
        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI("GAP", "BLE_GAP_EVENT_ADV_COMPLETE");
            ble_app_advertise();
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
ble_app_advertise (void)
{
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));
    
    // Generic discover or limited discover.
    //
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_DISC_LTD;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    fields.name = (uint8_t *) ble_svc_gap_device_name();
    fields.name_len = strlen(ble_svc_gap_device_name());
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    // We use a random address, no direct addressing, ble continuous advertising.
    //
    ble_gap_adv_start(g_ble_add_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

static void
ble_app_on_sync (void)
{
    // Automatically creates the best address.
    //
    ble_hs_id_infer_auto(0, &g_ble_add_type);
    ble_app_advertise();
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
