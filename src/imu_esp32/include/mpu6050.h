#ifndef MPU6050_H
#define MPU6050_H

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


// --- Struct Definition ---
typedef struct {
    float accel_x_g, accel_y_g, accel_z_g;
    float gyro_x_dps, gyro_y_dps, gyro_z_dps;
    float pitch_accel, roll_accel; // Calculated purely from gravity
} SensorData;

// Global Calibration Offsets


float Map(float x, float in_min, float in_max, float out_min, float out_max);
void ReadSensorData(i2c_master_dev_handle_t dev_handle, uint8_t* buffer);
void CalibrateGyro(i2c_master_dev_handle_t dev_handle);
void InitMPU6050(i2c_master_dev_handle_t dev_handle);




#endif