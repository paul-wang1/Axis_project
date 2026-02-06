/* * * * * * * MAIN.C * * * * * * * /
 * Main file for the ESP32 inside the guitar pedal.
 * Bluetooth interface file is within ble_interface.h
 * * * * * * * * * * * * * * * * * */

#include "esp_log.h"
#include "nvs_flash.h"
#include "ble_interface.h"

#define PEER_ADDR_VAL_SIZE      6



void app_main(void)
{
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
    ble_spp_uart_init();

    /* Configure the host. */
    ble_hs_cfg.reset_cb = ble_spp_client_on_reset;
    ble_hs_cfg.sync_cb = ble_spp_client_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Initialize data structures to track connected peers. */
    rc = peer_init(MYNEWT_VAL(BLE_MAX_CONNECTIONS), 64, 64, 64);
    assert(rc == 0);
    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set("axis_labs_pedal");
    assert(rc == 0);

    /* XXX Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(ble_spp_client_host_task);
}