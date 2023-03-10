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

static void
ble_app_on_sync (void)
{
    ble_addr_t address = {0};

    // Random address.
    //
    ble_hs_id_gen_rnd(1, &address);
    ble_hs_id_set_rnd(address.val);

    struct ble_hs_adv_fields fields = (struct ble_hs_adv_fields) {0};
    
    // Create an URL connected to the beacon.
    //
    ble_eddystone_set_adv_data_url(&fields, BLE_EDDYSTONE_URL_SCHEME_HTTPS, "learnesp32",
                                   strlen("learnesp32"), BLE_EDDYSTONE_URL_SUFFIX_COM, -30);

    struct ble_gap_adv_params adv_params = (struct ble_gap_adv_params) {0};

    // We use a random address, no direct addressing, ble continuous advertising.
    //
    ble_gap_adv_start(BLE_OWN_ADDR_RANDOM, NULL, BLE_HS_FOREVER, &adv_params, NULL, NULL);
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

    // Already initialized.
    //
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    
    // Nible has a particular task.
    //
    nimble_port_freertos_init(task_host);
}
