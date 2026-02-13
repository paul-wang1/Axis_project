#include "ble_interface.h"
#include "mpu6050.h"
const float ALPHA = 0.98f; // Complementary Filter: 98% Gyro, 2% Accel
void
app_main(void)
{
    // 1. Setup I2C
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MPU6050_SENSOR_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));

    // 2. Initialize Sensor
    InitMPU6050(dev_handle);
    
    // 3. Calibrate (Keep sensor still!)
    CalibrateGyro(dev_handle);

    // 4. Variables for Filter
    float filtered_pitch = 0.0f;
    float filtered_roll = 0.0f;
    int64_t last_time = esp_timer_get_time(); // Time in microseconds

    int print_counter = 0;

    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ret = nimble_port_init();
    if (ret != ESP_OK) {
        MODLOG_DFLT(ERROR, "Failed to init nimble %d \n", ret);
        return;
    }

    /* Initialize connection_handle array */
    for (int i = 0; i <= CONFIG_BT_NIMBLE_MAX_CONNECTIONS; i++) {
         conn_handle_subscriber[i] = false;
    }

    /* Initialize uart driver and start uart task */
    ble_spp_uart_init();

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = ble_spp_server_on_reset;
    ble_hs_cfg.sync_cb = ble_spp_server_on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_hs_cfg.sm_io_cap = CONFIG_EXAMPLE_IO_TYPE;
#ifdef CONFIG_EXAMPLE_BONDING
    ble_hs_cfg.sm_bonding = 1;
#endif
#ifdef CONFIG_EXAMPLE_MITM
    ble_hs_cfg.sm_mitm = 1;
#endif
#ifdef CONFIG_EXAMPLE_USE_SC
    ble_hs_cfg.sm_sc = 1;
#else
    ble_hs_cfg.sm_sc = 0;
#endif
#ifdef CONFIG_EXAMPLE_BONDING
    ble_hs_cfg.sm_our_key_dist = 1;
    ble_hs_cfg.sm_their_key_dist = 1;
#endif

#if MYNEWT_VAL(BLE_GATTS)
    int rc;
    /* Register custom service */
    rc = gatt_svr_init();
    assert(rc == 0);

    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set("Daisy Server");
    assert(rc == 0);
#endif

    /* XXX Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(ble_spp_server_host_task);
    
    // 5. Main Loopaccel_x
    while(1) {
        // A. Calculate Delta Time (dt) accurately
        int64_t current_time = esp_timer_get_time();
        float dt = (current_time - last_time) / 1000000.0f; // Convert us to seconds
        last_time = current_time;

        // B. Read Data
        SensorData data = ReadSensorData(dev_handle);

        // C. COMPLEMENTARY FILTER (The Fix for Drift)
        // Formula: Angle = 0.98 * (Angle + Gyro_Rate * dt) + 0.02 * Accel_Angle
        
        filtered_pitch = ALPHA * (filtered_pitch + data.gyro_y_dps * dt) + (1.0f - ALPHA) * data.pitch_accel;
        filtered_roll  = ALPHA * (filtered_roll  + data.gyro_x_dps * dt) + (1.0f - ALPHA) * data.roll_accel;

        // D. Mapping (Corrected Range)
        // Previous code mapped -45..45 to 0..180. This caused "dead zones" past 45 degrees.
        // New map: -90..90 to 0..180 (Full 180 degree motion)
        float output_pitch = Map(filtered_pitch, -90.0f, 90.0f, 0.0f, 180.0f);
        float output_roll  = Map(filtered_roll,  -90.0f, 90.0f, 0.0f, 180.0f);

        // // Print every 100ms
        // if (print_counter++ >= 10) { 
        //     // Note: filtered_pitch/roll are the REAL angles. output_pitch/roll are mapped 0-180.
        //     ESP_LOGI(TAG, "Pitch: %.2f | Roll: %.2f", output_pitch, output_roll);
        //     print_counter = 0;
        // }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}