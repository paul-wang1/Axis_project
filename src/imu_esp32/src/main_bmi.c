#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include <math.h>
#include <stdbool.h>
#include "esp_timer.h"

static const char *TAG = "BMI270_FIXED";

// --- Configuration ---
#define I2C_MASTER_SCL_IO           9
#define I2C_MASTER_SDA_IO           8
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TIMEOUT_MS       1000

#define BMI270_SENSOR_ADDR          0x68
#define BMI270_CHIP_ID_REG          0x00
#define BMI270_PWR_CONF_REG         0x7C
#define BMI270_PWR_CTRL_REG         0x7D
#define BMI270_ACC_CONF_REG         0x40
#define BMI270_GYR_CONF_REG         0x42

// Data Registers (Starting at ACC_X_LSB 0x0C)
#define BMI270_DATA_ADDR            0x0C 

// Sensitivity Constants
const float GYRO_SENS_250DPS  = 131.0f;
const float ACCEL_SENS_2G     = 16384.0f;
const float ALPHA             = 0.98f; 

typedef struct {
    float ax, ay, az; // In Gs
    float gx, gy, gz; // In degrees per second
    float roll_acc, pitch_acc;
} SensorData;

// --- I2C Helpers ---
static esp_err_t BMI270_register_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len) {
    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS);
}

static esp_err_t BMI270_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data) {
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS);
}

// --- Initialization ---
void InitBMI270(i2c_master_dev_handle_t dev_handle) {
    uint8_t chip_id = 0;
    BMI270_register_read(dev_handle, BMI270_CHIP_ID_REG, &chip_id, 1);
    ESP_LOGI(TAG, "Chip ID: 0x%02X (Expected 0x24)", chip_id);

    // 1. Disable advanced power save to configure
    BMI270_register_write_byte(dev_handle, BMI270_PWR_CONF_REG, 0x00);
    vTaskDelay(pdMS_TO_TICKS(10));

    // 2. Set Accel to 100Hz, Normal Mode (0xA8)
    BMI270_register_write_byte(dev_handle, BMI270_ACC_CONF_REG, 0xA8);
    // 3. Set Gyro to 100Hz, Normal Mode (0xE9)
    BMI270_register_write_byte(dev_handle, BMI270_GYR_CONF_REG, 0xE9);

    // 4. Enable Accel and Gyro in Power Control
    BMI270_register_write_byte(dev_handle, BMI270_PWR_CTRL_REG, 0x0E);
    
    vTaskDelay(pdMS_TO_TICKS(50));
    ESP_LOGI(TAG, "BMI270 Initialized");
}

// --- Read Data ---
SensorData ReadSensorData(i2c_master_dev_handle_t dev_handle) {
    uint8_t buf[12]; // 6 bytes Accel, 6 bytes Gyro
    SensorData data = {0};

    // Read 12 bytes starting from ACC_X_LSB
    if (BMI270_register_read(dev_handle, BMI270_DATA_ADDR, buf, 12) == ESP_OK) {
        // Raw conversions (Little Endian)
        int16_t raw_ax = (int16_t)((buf[1] << 8) | buf[0]);
        int16_t raw_ay = (int16_t)((buf[3] << 8) | buf[2]);
        int16_t raw_az = (int16_t)((buf[5] << 8) | buf[4]);
        
        int16_t raw_gx = (int16_t)((buf[7] << 8) | buf[6]);
        int16_t raw_gy = (int16_t)((buf[9] << 8) | buf[8]);
        int16_t raw_gz = (int16_t)((buf[11] << 8) | buf[10]);

        // Convert to Gs and DPS
        data.ax = (float)raw_ax / ACCEL_SENS_2G;
        data.ay = (float)raw_ay / ACCEL_SENS_2G;
        data.az = (float)raw_az / ACCEL_SENS_2G;
        
        data.gx = (float)raw_gx / GYRO_SENS_250DPS;
        data.gy = (float)raw_gy / GYRO_SENS_250DPS;
        data.gz = (float)raw_gz / GYRO_SENS_250DPS;

        // Calculate Accelerometer-based Roll and Pitch
        data.roll_acc  = atan2f(data.ay, data.az) * 180.0f / M_PI;
        data.pitch_acc = atan2f(-data.ax, sqrtf(data.ay * data.ay + data.az * data.az)) * 180.0f / M_PI;
    }
    return data;
}

float Map(float x, float in_min, float in_max, float out_min, float out_max) {
    if (x < in_min) x = in_min;
    if (x > in_max) x = in_max;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void app_main(void) {
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
        .device_address = BMI270_SENSOR_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));

    InitBMI270(dev_handle);

    float roll = 0.0f, pitch = 0.0f, yaw = 0.0f;
    int64_t last_time = esp_timer_get_time();

    while(1) {
        int64_t current_time = esp_timer_get_time();
        float dt = (float)(current_time - last_time) / 1000000.0f;
        last_time = current_time;

        SensorData data = ReadSensorData(dev_handle);

        // Complementary Filter
        roll  = ALPHA * (roll + data.gx * dt) + (1.0f - ALPHA) * data.roll_acc;
        pitch = ALPHA * (pitch + data.gy * dt) + (1.0f - ALPHA) * data.pitch_acc;
        
        // Yaw (Gyro only - will drift!)
        yaw += data.gz * dt;

        // Map for output (e.g., 0-180 for servos or visualization)
        float out_roll  = Map(roll, -90.0f, 90.0f, 0.0f, 180.0f);
        float out_pitch = Map(pitch, -90.0f, 90.0f, 0.0f, 180.0f);

        ESP_LOGI(TAG, "R: %.1f | P: %.1f | Y: %.1f", out_roll, out_pitch, yaw);

        vTaskDelay(pdMS_TO_TICKS(20)); // ~50Hz update rate
    }
}
