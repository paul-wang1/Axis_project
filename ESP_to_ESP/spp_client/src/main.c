/* * * * * * * MAIN.C * * * * * * * /
 * Main file for the ESP32 inside the guitar pedal.
 * Bluetooth interface file is within ble_interface.h
 * * * * * * * * * * * * * * * * * */

#include "esp_log.h"
#include "nvs_flash.h"
#include "ble_interface.h"
#include "i2c.h"

#define PEER_ADDR_VAL_SIZE      6

static const char *TAG = "app_main";

void app_main(void)
{
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1 << GPIO_NUM_8 ;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    // bool logic_level = !logic_level;
        gpio_set_level(GPIO_NUM_8, 1);
    int rc;
    /* Initialize NVS — it is used to store PHY calibration data */
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

    /* Initialize UART driver and start uart task */
    // ble_axis_uart_init();

    /* Configure the host. */
    ble_hs_cfg.reset_cb = ble_axis_client_on_reset;
    ble_hs_cfg.sync_cb = ble_axis_client_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Initialize data structures to track connected peers. */
    rc = peer_init(MYNEWT_VAL(BLE_MAX_CONNECTIONS), 64, 64, 64);
    assert(rc == 0);
    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set("axis_labs_pedal");
    assert(rc == 0);

    /* XXX Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(ble_axis_client_host_task);
        /* Create command queue */
    cmd_queue = xQueueCreate(16, sizeof(uint8_t));
    if (cmd_queue == NULL)
    {
        // ESP_LOGE(TAG, "Failed to create command queue");
        return;
    }

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
        // ESP_LOGE(TAG, "Failed to create I2C slave device: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "I2C slave device created");

    /* Register callbacks */
    i2c_slave_event_callbacks_t cbs = {
        .on_recv_done = i2c_slave_recv_done_cb,
    };
    err = i2c_slave_register_event_callbacks(i2c_slave_handle, &cbs, NULL);
    if (err != ESP_OK)
    {
        // ESP_LOGE(TAG, "Failed to register callbacks: %s", esp_err_to_name(err));
        return;
    }
    // ESP_LOGI(TAG, "I2C callbacks registered");

    /* Pre-load default data (Value A) */
    load_response_data(CMD_GET_VALUE_A);

    /* Start command handler task */
    xTaskCreate(cmd_handler_task, "cmd_handler", 4096, NULL, 10, NULL);
}