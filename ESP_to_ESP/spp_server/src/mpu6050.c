#include "mpu6050.h"

// Sensitivity (defaults)

static const float GYRO_SENSITIVITY = 131.0f;
// static const float ACCEL_SENSITIVITY = 16384.0f;
static float gyro_x_offset = 0.0f;
static float gyro_y_offset = 0.0f;
static float gyro_z_offset = 0.0f;


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
void ReadSensorData(i2c_master_dev_handle_t dev_handle, uint8_t *buffer) {

    mpu6050_register_read(dev_handle, MPU6050_ACCEL_XOUT_H, buffer, 14);

    // SensorData data;
    
    // // Raw Values
    // int16_t raw_ax = (int16_t)((buffer[0] << 8) | buffer[1]);
    // int16_t raw_ay = (int16_t)((buffer[2] << 8) | buffer[3]);
    // int16_t raw_az = (int16_t)((buffer[4] << 8) | buffer[5]);
    // int16_t raw_gx = (int16_t)((buffer[8] << 8) | buffer[9]);
    // int16_t raw_gy = (int16_t)((buffer[10] << 8) | buffer[11]);
    // int16_t raw_gz = (int16_t)((buffer[12] << 8) | buffer[13]);

    // // Convert to physical units
    // data.accel_x_g = raw_ax / ACCEL_SENSITIVITY;
    // data.accel_y_g = raw_ay / ACCEL_SENSITIVITY;
    // data.accel_z_g = raw_az / ACCEL_SENSITIVITY;

    // // Apply Calibration Offsets to Gyro
    // data.gyro_x_dps = (raw_gx / GYRO_SENSITIVITY) - gyro_x_offset;
    // data.gyro_y_dps = (raw_gy / GYRO_SENSITIVITY) - gyro_y_offset;
    // data.gyro_z_dps = (raw_gz / GYRO_SENSITIVITY) - gyro_z_offset;

    // // Calculate Accelerometer Angles (The "Stable" but Noisy values)
    // // Note: atan2 returns radians, convert to degrees
    // data.roll_accel = atan2(data.accel_y_g, data.accel_z_g) * 180.0f / M_PI;
    // data.pitch_accel = atan2(-data.accel_x_g, sqrt(data.accel_y_g * data.accel_y_g + data.accel_z_g * data.accel_z_g)) * 180.0f / M_PI;

    // return data;
}

// Map function (Improved to protect against division by zero)
float Map(float x, float in_min, float in_max, float out_min, float out_max) {
    // Clamp input to bounds
    if (x < in_min) x = in_min;
    if (x > in_max) x = in_max;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}