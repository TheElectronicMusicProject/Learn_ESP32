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
#include "services/gatt/ble_svc_gatt.h"
#include <stdio.h>

#define DEVICE_NAME                 "MY BLE DEVICE"
#define DEVICE_INFOR_SERVICE        0x180A
#define MANUFACTURER_NAME           0X2A29
#define BATTERY_SERVICE             0x180F
#define BATTERY_LEVEL_CHAR          0X2A19
#define BATTERY_CHAR_CONFIG_DESC    0x2902

static uint8_t g_ble_add_type = 0;
static xTimerHandle gh_timer = NULL;
static uint8_t g_battery_level = 100;
static uint16_t gh_battery_char_att = 0;
static uint16_t gh_connection = 0;

// Enabled or disabled.
//
static uint8_t g_config[2] = {0x01, 0x00};

static int
device_info (uint16_t conn_handle, uint16_t attr_handle,
             struct ble_gatt_access_ctxt * p_ctxt, void * p_arg)
{
    // mbuf is a buffer used by nible bluetooth that keep tracks of data.
    //
    os_mbuf_append(p_ctxt->om, "manufacturer name", strlen("manufacturer name"));

    return 0;
}

static int
device_write (uint16_t conn_handle, uint16_t attr_handle,
             struct ble_gatt_access_ctxt * p_ctxt, void * p_arg)
{
    printf("incoming message: %.*s\n", p_ctxt->om->om_len, p_ctxt->om->om_data);

    return 0;
}

static int
battery_read (uint16_t conn_handle, uint16_t attr_handle,
             struct ble_gatt_access_ctxt * p_ctxt, void * p_arg)
{
    uint8_t battery_level = 85;

    os_mbuf_append(p_ctxt->om, &battery_level, sizeof(battery_level));

    return 0;
}

static int
battery_descriptor (uint16_t conn_handle, uint16_t attr_handle,
                   struct ble_gatt_access_ctxt * p_ctxt, void * p_arg)
{
    if (BLE_GATT_ACCESS_OP_READ_DSC == p_ctxt->op)
    {
        os_mbuf_append(p_ctxt->om, &g_config, sizeof(g_config));
    }
    else
    {
        memcpy(g_config, p_ctxt->om->om_data, p_ctxt->om->om_len);
    }

    if (0x01 == g_config[0])
    {
        xTimerStart(gh_timer, 0);
    }
    else
    {
        xTimerStop(gh_timer, 0);
    }

    return 0;
}

const struct ble_gatt_svc_def g_gatt_svcs[] = {
    {
        // Primary services.
        //
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        // From the Bluetooth documentation, we choose device informations.
        //
        .uuid = BLE_UUID16_DECLARE(DEVICE_INFOR_SERVICE),
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = BLE_UUID16_DECLARE(MANUFACTURER_NAME),
                .flags = BLE_GATT_CHR_F_READ,
                .access_cb = device_info
            },
            {
                .uuid = BLE_UUID128_DECLARE(0x00, 0x11, 0x22, 0x33,
                                            0x44, 0x55, 0x66, 0x77,
                                            0x88, 0x99, 0XAA, 0XBB,
                                            0XCC, 0XDD, 0XEE, 0XFF),
                .flags = BLE_GATT_CHR_F_WRITE,
                .access_cb = device_write
            },
            {
                0
            }
        }
    },
    {
        // Primary services.
        //
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        // From the Bluetooth documentation, we choose device informations.
        //
        .uuid = BLE_UUID16_DECLARE(BATTERY_SERVICE),
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = BLE_UUID16_DECLARE(BATTERY_LEVEL_CHAR),
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .access_cb = battery_read,
                .val_handle = &gh_battery_char_att,
                .descriptors = (struct ble_gatt_dsc_def[])
                {
                    {
                        .uuid = BLE_UUID16_DECLARE(BATTERY_CHAR_CONFIG_DESC),
                        .att_flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                        .access_cb = battery_descriptor
                    },
                    {
                        0
                    }
                }
            },
            {
                0
            }
        }
    },
    {
        0
    }
};

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

            gh_connection = p_event->connect.conn_handle;
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

            if (p_event->subscribe.attr_handle == gh_battery_char_att)
            {
                xTimerStart(gh_timer, 0);
            }
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

static void
timer_update_status (TimerHandle_t h_timer)
{
    if (0 == g_battery_level--)
    {
        g_battery_level = 100;
    }

    printf("notify battery level is %d\n", g_battery_level);

    struct os_mbuf * p_om = ble_hs_mbuf_from_flat(&g_battery_level, sizeof(g_battery_level));
    ble_gattc_notify_custom(gh_connection, gh_battery_char_att, p_om);
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
    ble_svc_gatt_init();

    // For memory under the hoods.
    //
    ble_gatts_count_cfg(g_gatt_svcs);
    ble_gatts_add_svcs(g_gatt_svcs);

    gh_timer = xTimerCreate("update_battery_status", pdMS_TO_TICKS(1000), pdTRUE, NULL, timer_update_status);

    // Already initialized.
    //
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    
    // Nible has a particular task.
    //
    nimble_port_freertos_init(task_host);
}
