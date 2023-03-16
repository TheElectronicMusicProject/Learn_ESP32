#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"

#define SDA_GPIO    26
#define SCL_GPIO    25
#define LM75A_ADDR  0x48
#define TAG         "i2c"

void
app_main (void)
{
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_GPIO,
        .scl_io_num = SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    uint8_t raw[2] = {0};
    i2c_cmd_handle_t h_cmd = i2c_cmd_link_create();
    i2c_master_start(h_cmd);
    i2c_master_write_byte(h_cmd, (LM75A_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(h_cmd, (uint8_t *) &raw, 2, I2C_MASTER_ACK);
    i2c_master_stop(h_cmd);
    i2c_master_cmd_begin(I2C_NUM_0, h_cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(h_cmd);
    h_cmd = NULL;

    bool b_is_negative = false;

    if (0 != (raw[0] & 0x80))
    {
        b_is_negative = true;
        raw[0] &= 0x7F;
    }

    int16_t data = (raw[0] << 8 | raw[1]) >> 5;
    float temperature = data * 0.125;
    temperature *= b_is_negative ? -1 : 1;

    printf("raw[0] %x raw[1] %x == data %x\n", raw[0], raw[1], data);
    printf("temperature %f\n", temperature);

    ESP_ERROR_CHECK(i2c_driver_delete(I2C_NUM_0));
}
