/* * * * * * * MAIN.C * * * * * * * /
 * Main file for the ESP32 inside the guitar pedal.
 * Bluetooth interface file is within ble_interface.h
 * * * * * * * * * * * * * * * * * */

#include "esp_log.h"
#include "driver/i2c_master.h"
#include "i2c.h"

/* BLE */
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "modlog/modlog.h"
#include "esp_central.h"
#include "ble_interface.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "services/gap/ble_svc_gap.h"

#define PEER_ADDR_VAL_SIZE      6

static const char *TAG = "app_main";
i2c_slave_dev_handle_t i2c_slave_handle = NULL;
i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t dev_handle;

// Change these to your pins
#define I2C_MASTER_SCL_IO       9
#define I2C_MASTER_SDA_IO       8
#define I2C_MASTER_NUM          I2C_NUM_0
#define I2C_MASTER_FREQ_HZ      100000


// 7-bit address
#define DEVICE_ADDR             0x10

void app_main(void)
{
    /* Initialize NVS — it is used to store PHY calibration data and data for Bluetooth */
    esp_err_t ret = nvs_flash_init();
    if  (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ret = nimble_port_init();
    if (ret != ESP_OK) {
        MODLOG_DFLT(ERROR, "Failed to init nimble %d \n", ret);
        return;
    }

    /* Configure the host. */
    ble_hs_cfg.reset_cb = ble_axis_client_on_reset;
    ble_hs_cfg.sync_cb = ble_axis_client_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Initialize data structures to track connected peers. */
    int rc;
    rc = peer_init(MYNEWT_VAL(BLE_MAX_CONNECTIONS), 64, 64, 64);
    assert(rc == 0);
    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set("axis_labs_pedal");
    assert(rc == 0);

    /* XXX Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(ble_axis_client_host_task);


   // Configure I2C bus
   i2c_master_bus_config_t bus_config = {
       .i2c_port = I2C_MASTER_NUM,
       .sda_io_num = I2C_MASTER_SDA_IO,
       .scl_io_num = I2C_MASTER_SCL_IO,
       .clk_source = I2C_CLK_SRC_DEFAULT,
       .glitch_ignore_cnt = 7,
       .flags.enable_internal_pullup = false,
   };


   ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));


   // Configure I2C device
   i2c_device_config_t dev_config = {
       .dev_addr_length = I2C_ADDR_BIT_LEN_7,
       .device_address = DEVICE_ADDR,
       .scl_speed_hz = I2C_MASTER_FREQ_HZ,
   };


   ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));

}
