#include "MPU6050.h"
#include "daisy_seed.h"
#include "daisysp.h"
#include <math.h>

using namespace daisy;
using namespace daisysp;

I2CHandle i2c;

// MPU6050 I2C Address (AD0 pin = LOW)
const uint8_t MPU6050_ADDRESS = 0x68;

// MPU6050 Register Addresses
const uint8_t MPU6050_WHO_AM_I = 0x75;
const uint8_t MPU6050_PWR_MGMT_1 = 0x6B;
const uint8_t MPU6050_PWR_MGMT_2 = 0x6C;
const uint8_t MPU6050_GYRO_CONFIG = 0x1B;
const uint8_t MPU6050_ACCEL_CONFIG = 0x1C;
const uint8_t MPU6050_ACCEL_XOUT_H = 0x3B;

// Sensitivity values (for ±250°/s and ±2g)
const float GYRO_SENSITIVITY = 131.0f;   // LSB per °/s for ±250°/s range
const float ACCEL_SENSITIVITY = 16384.0f; // LSB per g for ±2g range

// Helper functions for I2C communication
void WriteMPU6050Register(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    i2c.TransmitBlocking(MPU6050_ADDRESS, buffer, 2, 100);
}

void ReadMPU6050Register(uint8_t reg, uint8_t arr[], uint8_t size) {
    i2c.TransmitBlocking(MPU6050_ADDRESS, &reg, 1, 100);
    i2c.ReceiveBlocking(MPU6050_ADDRESS, arr, size, 100);
}

void InitMPU6050() {
    // Configure I2C for MPU6050
    I2CHandle::Config i2c_conf;
    i2c_conf.periph = I2CHandle::Config::Peripheral::I2C_1;
    i2c_conf.mode = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_conf.speed = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_conf.pin_config.scl = seed::D11;
    i2c_conf.pin_config.sda = seed::D12;
    
    // Initialize I2C
    if (i2c.Init(i2c_conf) != I2CHandle::Result::OK) {
        // I2C initialization failed - handle error
        while(1) System::Delay(100);
    }
    
    System::Delay(100); // Give sensor time to boot
    
    // Wake up MPU6050 (it starts in sleep mode)
    WriteMPU6050Register(MPU6050_PWR_MGMT_1, 0x00);
    System::Delay(100);
    
    // Set clock source to PLL with X-axis gyroscope reference (more stable)
    WriteMPU6050Register(MPU6050_PWR_MGMT_1, 0x01);
    System::Delay(10);
    
    // Configure Gyroscope (±250°/s range)
    WriteMPU6050Register(MPU6050_GYRO_CONFIG, 0x00);
    System::Delay(10);
    
    // Configure Accelerometer (±2g range)
    WriteMPU6050Register(MPU6050_ACCEL_CONFIG, 0x00);
    System::Delay(10);
}

SensorData ReadSensorData() {
    uint8_t buffer[14];
    
    // Read all 14 bytes starting from ACCEL_XOUT_H
    // This reads: ACCEL_X, ACCEL_Y, ACCEL_Z, TEMP, GYRO_X, GYRO_Y, GYRO_Z
    ReadMPU6050Register(MPU6050_ACCEL_XOUT_H, buffer, 14);
    
    SensorData data;
    
    // Parse raw values (big-endian format)
    data.accel_x = (int16_t)((buffer[0] << 8) | buffer[1]);
    data.accel_y = (int16_t)((buffer[2] << 8) | buffer[3]);
    data.accel_z = (int16_t)((buffer[4] << 8) | buffer[5]);
    // buffer[6-7] is temperature (we're skipping it)
    data.gyro_x = (int16_t)((buffer[8] << 8) | buffer[9]);
    data.gyro_y = (int16_t)((buffer[10] << 8) | buffer[11]);
    data.gyro_z = (int16_t)((buffer[12] << 8) | buffer[13]);
    
    // Convert to physical units
    data.accel_x_g = data.accel_x / ACCEL_SENSITIVITY;
    data.accel_y_g = data.accel_y / ACCEL_SENSITIVITY;
    data.accel_z_g = data.accel_z / ACCEL_SENSITIVITY;
    
    data.gyro_x_dps = data.gyro_x / GYRO_SENSITIVITY;
    data.gyro_y_dps = data.gyro_y / GYRO_SENSITIVITY;
    data.gyro_z_dps = data.gyro_z / GYRO_SENSITIVITY;
    
    // Calculate roll and pitch from accelerometer
    // Note: These are static angles, not from sensor fusion like BNO055
    // Roll (rotation around X-axis)
    data.roll = atan2(data.accel_y_g, data.accel_z_g) * 180.0f / M_PI;
    
    // Pitch (rotation around Y-axis)
    data.pitch = atan2(-data.accel_x_g, sqrt(data.accel_y_g * data.accel_y_g + 
                                              data.accel_z_g * data.accel_z_g)) * 180.0f / M_PI;
    
    // Note: Heading/yaw cannot be calculated without a magnetometer
    
    return data;
}

// Test if MPU6050 is connected
bool TestMPU6050() {
    uint8_t who_am_i = 0;
    ReadMPU6050Register(MPU6050_WHO_AM_I, &who_am_i, 1);
    return (who_am_i == 0x68);
}