// #include <stdio.h>
// #include "sdkconfig.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"
// #include "driver/i2c_master.h"
// #include <math.h>
// #include "mpu6050.h"
// #include <stdbool.h>

// static const char *TAG = "example";


// #define I2C_MASTER_SCL_IO   9   /*!< GPIO number used for I2C master clock */
// #define I2C_MASTER_SDA_IO   8   /*!< GPIO number used for I2C master data  */
// #define I2C_MASTER_NUM              I2C_NUM_0                   /*!< I2C port number for master dev */
// #define I2C_MASTER_FREQ_HZ  400000 // 400kHz (Fast Mode)
// #define I2C_MASTER_TX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
// #define I2C_MASTER_RX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
// #define I2C_MASTER_TIMEOUT_MS       1000

// // MPU6050 Register Addresses
// const uint8_t MPU6050_SENSOR_ADDR = 0x68; //AD0 must be low
// const uint8_t MPU6050_WHO_AM_I_REG_ADDR = 0x75;
// const uint8_t MPU6050_PWR_MGMT_1_REG_ADDR = 0x6B;
// const uint8_t MPU6050_GYRO_CONFIG = 0x1B;
// const uint8_t MPU6050_ACCEL_CONFIG = 0x1C;
// const uint8_t MPU6050_ACCEL_XOUT_H = 0x3B;

// // Sensitivity values (for ±250°/s and ±2g)
// const float GYRO_SENSITIVITY = 131.0f;   // LSB per °/s for ±250°/s range
// const float ACCEL_SENSITIVITY = 16384.0f; // LSB per g for ±2g range

// /**
//  * @brief Read a sequence of bytes from a MPU6050 sensor registers
//  */
// static esp_err_t mpu6050_register_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len)
// {
//     return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
// }

// /**
//  * @brief Write a byte to a MPU6050 sensor register
//  */
// static esp_err_t mpu6050_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data)
// {
//     uint8_t write_buf[2] = {reg_addr, data};
//     return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
// }

// /*
//  * Initalize MPU-6050
//  */
// void InitMPU6050(i2c_master_dev_handle_t dev_handle) {
//     mpu6050_register_write_byte(dev_handle, MPU6050_PWR_MGMT_1_REG_ADDR, 0x00);
//     vTaskDelay(pdMS_TO_TICKS(100));
//     mpu6050_register_write_byte(dev_handle, MPU6050_PWR_MGMT_1_REG_ADDR, 0x01);
//     vTaskDelay(pdMS_TO_TICKS(10));
//     mpu6050_register_write_byte(dev_handle, MPU6050_GYRO_CONFIG, 0x00);
//     vTaskDelay(pdMS_TO_TICKS(10));
//     mpu6050_register_write_byte(dev_handle, MPU6050_ACCEL_CONFIG, 0x00);
//     vTaskDelay(pdMS_TO_TICKS(10));
// }

// /*
//  *  Read MPU-6050 Data
//  */
// SensorData ReadSensorData(i2c_master_dev_handle_t dev_handle) {
//     uint8_t buffer[14];

//     mpu6050_register_read(dev_handle, MPU6050_ACCEL_XOUT_H, buffer, 14);

//     SensorData data;
    
//     // Parse raw values (big-endian format)
//     data.accel_x = (int16_t)((buffer[0] << 8) | buffer[1]);
//     data.accel_y = (int16_t)((buffer[2] << 8) | buffer[3]);
//     data.accel_z = (int16_t)((buffer[4] << 8) | buffer[5]);
//     // buffer[6-7] is temperature (we're skipping it)
//     data.gyro_x = (int16_t)((buffer[8] << 8) | buffer[9]);
//     data.gyro_y = (int16_t)((buffer[10] << 8) | buffer[11]);
//     data.gyro_z = (int16_t)((buffer[12] << 8) | buffer[13]);
    
//     // Convert to physical units
//     data.accel_x_g = data.accel_x / ACCEL_SENSITIVITY;
//     data.accel_y_g = data.accel_y / ACCEL_SENSITIVITY;
//     data.accel_z_g = data.accel_z / ACCEL_SENSITIVITY;
    
//     data.gyro_x_dps = data.gyro_x / GYRO_SENSITIVITY;
//     data.gyro_y_dps = data.gyro_y / GYRO_SENSITIVITY;
//     data.gyro_z_dps = data.gyro_z / GYRO_SENSITIVITY;

//     // Roll (rotation around X-axis)
//     data.roll = atan2(data.accel_y_g, data.accel_z_g) * 180.0f / M_PI;
    
//     // Pitch (rotation around Y-axis)
//     data.pitch = atan2(-data.accel_x_g, sqrt(data.accel_y_g * data.accel_y_g + 
//                                               data.accel_z_g * data.accel_z_g)) * 180.0f / M_PI;

//     return data;
// }

// bool TestMPU6050(i2c_master_dev_handle_t dev_handle) {
//     uint8_t who_am_i = 0;
//     mpu6050_register_read(dev_handle, MPU6050_WHO_AM_I_REG_ADDR, &who_am_i, 1);
//     return (who_am_i == 0x68);
// }

// float Map(float x, float in_min, float in_max, float out_min, float out_max) {
//     if (in_max == in_min) return out_min; // Avoid division by zero
//     x = fminf(fmaxf(x, in_min), in_max);
//     return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
// }

// /**
//  * @brief i2c master initialization
//  */
// static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle)
// {
//     i2c_master_bus_config_t bus_config = {
//         .i2c_port = I2C_MASTER_NUM,
//         .sda_io_num = I2C_MASTER_SDA_IO,
//         .scl_io_num = I2C_MASTER_SCL_IO,
//         .clk_source = I2C_CLK_SRC_DEFAULT,
//         .glitch_ignore_cnt = 7,
//         .flags.enable_internal_pullup = true,
//     };
//     ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));

//     i2c_device_config_t dev_config = {
//         .dev_addr_length = I2C_ADDR_BIT_LEN_7,
//         .device_address = MPU6050_SENSOR_ADDR,
//         .scl_speed_hz = I2C_MASTER_FREQ_HZ,
//     };
//     ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
// }

// void app_main(void)
// {
//     i2c_master_bus_handle_t bus_handle;
//     i2c_master_dev_handle_t dev_handle;
//     i2c_master_init(&bus_handle, &dev_handle);
//     ESP_LOGI(TAG, "I2C initialized successfully");

//     if (!TestMPU6050(dev_handle)) {
//         ESP_LOGI(TAG, "WHO_AM_I data is incorrect");
//     } else {
//         ESP_LOGI(TAG, "WHO_AM_I data is correct");

//         InitMPU6050(dev_handle);
//         SensorData MPU_data;
//         int i = 0;
//         while(1) {
//             MPU_data = ReadSensorData(dev_handle);
//             float pitch = Map(MPU_data.pitch, -45.0f, 45.0f, 0.0f, 180.0f);
//             float roll = Map(MPU_data.roll, -45.0f, 45.0f, 0.0f, 180.0f);

//             if (i == 200) {
//                 ESP_LOGI(TAG, "Pitch: %f", pitch);
//                 ESP_LOGI(TAG, "Roll: %f", roll);
//                 i = 0;
//             }
//             i = i+1;

//             // ESP_LOGI(TAG, "X acceleration = %f", MPU_data.accel_x_g);
//             // ESP_LOGI(TAG, "Y acceleration = %f", MPU_data.accel_y_g);
//             // ESP_LOGI(TAG, "Z acceleration = %f", MPU_data.accel_z_g);

//             // ESP_LOGI(TAG, "X gryo = %f", MPU_data.gyro_x_dps);
//             // ESP_LOGI(TAG, "Y gryo = %f", MPU_data.gyro_y_dps);
//             // ESP_LOGI(TAG, "Z gryo = %f", MPU_data.gyro_z_dps);

//             vTaskDelay(pdMS_TO_TICKS(10));
//         }
//     }
    
//     // /* Read the MPU6050 WHO_AM_I register, on power up the register should have the value 0x71 */
//     // ESP_ERROR_CHECK(mpu6050_register_read(dev_handle, MPU6050_WHO_AM_I_REG_ADDR, data, 1));
//     // ESP_LOGI(TAG, "WHO_AM_I = %X", data[0]);

//     // /* Demonstrate writing by resetting the MPU6050 */
//     // // ESP_ERROR_CHECK(mpu6050_register_write_byte(dev_handle, MPU6050_PWR_MGMT_1_REG_ADDR, 1 << MPU6050_RESET_BIT));

//     ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle));
//     ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));
//     ESP_LOGI(TAG, "I2C de-initialized successfully");
// }

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include <math.h>
#include <stdbool.h>
#include "esp_timer.h" // Needed for accurate dt

static const char *TAG = "MPU6050_FIXED";

// --- Configuration ---
#define I2C_MASTER_SCL_IO           9
#define I2C_MASTER_SDA_IO           8
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TIMEOUT_MS       1000

#define MPU6050_SENSOR_ADDR         0x68
#define MPU6050_PWR_MGMT_1_REG_ADDR 0x6B
#define MPU6050_ACCEL_XOUT_H        0x3B
#define MPU6050_GYRO_CONFIG         0x1B
#define MPU6050_ACCEL_CONFIG        0x1C

// Sensitivity (defaults)
const float GYRO_SENSITIVITY = 131.0f;
const float ACCEL_SENSITIVITY = 16384.0f;
const float ALPHA = 0.98f; // Complementary Filter: 98% Gyro, 2% Accel

// --- Struct Definition ---
typedef struct {
    float accel_x_g, accel_y_g, accel_z_g;
    float gyro_x_dps, gyro_y_dps, gyro_z_dps;
    float pitch_accel, roll_accel; // Calculated purely from gravity
} SensorData;

// Global Calibration Offsets
float gyro_x_offset = 0.0f;
float gyro_y_offset = 0.0f;
float gyro_z_offset = 0.0f;

// --- I2C Helpers ---
static esp_err_t mpu6050_register_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len) {
    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

static esp_err_t mpu6050_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data) {
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

// --- Initialization ---
void InitMPU6050(i2c_master_dev_handle_t dev_handle) {
    // Wake up
    mpu6050_register_write_byte(dev_handle, MPU6050_PWR_MGMT_1_REG_ADDR, 0x00);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Config Gyro (±250°/s) and Accel (±2g)
    mpu6050_register_write_byte(dev_handle, MPU6050_GYRO_CONFIG, 0x00);
    mpu6050_register_write_byte(dev_handle, MPU6050_ACCEL_CONFIG, 0x00);
    ESP_LOGI(TAG, "MPU6050 Woken Up");
}

// --- Calibration Routine ---
// MUST be called while sensor is sitting flat and still!
void CalibrateGyro(i2c_master_dev_handle_t dev_handle) {
    ESP_LOGI(TAG, "Calibrating Gyro... KEEP STILL!");
    float x_sum = 0, y_sum = 0, z_sum = 0;
    int samples = 500;
    uint8_t buffer[14];

    for (int i = 0; i < samples; i++) {
        mpu6050_register_read(dev_handle, MPU6050_ACCEL_XOUT_H, buffer, 14);
        x_sum += (int16_t)((buffer[8] << 8) | buffer[9]);
        y_sum += (int16_t)((buffer[10] << 8) | buffer[11]);
        z_sum += (int16_t)((buffer[12] << 8) | buffer[13]);
        vTaskDelay(pdMS_TO_TICKS(2));
    }

    gyro_x_offset = (x_sum / samples) / GYRO_SENSITIVITY;
    gyro_y_offset = (y_sum / samples) / GYRO_SENSITIVITY;
    gyro_z_offset = (z_sum / samples) / GYRO_SENSITIVITY;
    ESP_LOGI(TAG, "Calibration Done. Offsets X:%.2f Y:%.2f Z:%.2f", gyro_x_offset, gyro_y_offset, gyro_z_offset);
}

// --- Read Data ---
SensorData ReadSensorData(i2c_master_dev_handle_t dev_handle) {
    uint8_t buffer[14];
    mpu6050_register_read(dev_handle, MPU6050_ACCEL_XOUT_H, buffer, 14);

    SensorData data;
    
    // Raw Values
    int16_t raw_ax = (int16_t)((buffer[0] << 8) | buffer[1]);
    int16_t raw_ay = (int16_t)((buffer[2] << 8) | buffer[3]);
    int16_t raw_az = (int16_t)((buffer[4] << 8) | buffer[5]);
    int16_t raw_gx = (int16_t)((buffer[8] << 8) | buffer[9]);
    int16_t raw_gy = (int16_t)((buffer[10] << 8) | buffer[11]);
    int16_t raw_gz = (int16_t)((buffer[12] << 8) | buffer[13]);

    // Convert to physical units
    data.accel_x_g = raw_ax / ACCEL_SENSITIVITY;
    data.accel_y_g = raw_ay / ACCEL_SENSITIVITY;
    data.accel_z_g = raw_az / ACCEL_SENSITIVITY;

    // Apply Calibration Offsets to Gyro
    data.gyro_x_dps = (raw_gx / GYRO_SENSITIVITY) - gyro_x_offset;
    data.gyro_y_dps = (raw_gy / GYRO_SENSITIVITY) - gyro_y_offset;
    data.gyro_z_dps = (raw_gz / GYRO_SENSITIVITY) - gyro_z_offset;

    // Calculate Accelerometer Angles (The "Stable" but Noisy values)
    // Note: atan2 returns radians, convert to degrees
    data.roll_accel = atan2(data.accel_y_g, data.accel_z_g) * 180.0f / M_PI;
    data.pitch_accel = atan2(-data.accel_x_g, sqrt(data.accel_y_g * data.accel_y_g + data.accel_z_g * data.accel_z_g)) * 180.0f / M_PI;

    return data;
}

// Map function (Improved to protect against division by zero)
float Map(float x, float in_min, float in_max, float out_min, float out_max) {
    // Clamp input to bounds
    if (x < in_min) x = in_min;
    if (x > in_max) x = in_max;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void app_main(void)
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

    // 5. Main Loop
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

        // Print every 100ms
        if (print_counter++ >= 10) { 
            // Note: filtered_pitch/roll are the REAL angles. output_pitch/roll are mapped 0-180.
            ESP_LOGI(TAG, "Pitch: %.2f | Roll: %.2f", output_pitch, output_roll);
            print_counter = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
