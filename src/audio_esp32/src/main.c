/* * * * * * * MAIN.C * * * * * * * /
 * Main file for the ESP32 inside the guitar pedal.
 * Bluetooth interface file is within ble_interface.h
 * * * * * * * * * * * * * * * * * */

#include "esp_log.h"
#include "i2c.h"

#define PEER_ADDR_VAL_SIZE      6

static const char *TAG = "app_main";
i2c_slave_dev_handle_t i2c_slave_handle = NULL;


void app_main(void)
{

    /* Configure I2C slave */
    i2c_slave_config_t i2c_slv_config = {
        .i2c_port = I2C_SLAVE_NUM,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .scl_io_num = I2C_SLAVE_SCL_IO,
        .sda_io_num = I2C_SLAVE_SDA_IO,
        .slave_addr = ESP_SLAVE_ADDR,
        .send_buf_depth = I2C_SLAVE_RAM_SIZE,
    };

    esp_err_t err = i2c_new_slave_device(&i2c_slv_config, &i2c_slave_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create I2C slave device: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "I2C slave device created");

    ESP_ERROR_CHECK(i2c_slave_init());
    uint8_t data_to_send[] = {0xAA, 0x55, 0x12, 0x34};
    i2c_slave_write(I2C_SLAVE_NUM, data_to_send, sizeof(data_to_send), 200);
    
}
