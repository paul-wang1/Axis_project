#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include "per/i2c.h"

// Structure to hold sensor readings
struct SensorData {
    // Raw accelerometer data
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    
    // Raw gyroscope data
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    
    // Converted values (in physical units)
    float accel_x_g;   // g's
    float accel_y_g;
    float accel_z_g;
    
    float gyro_x_dps;  // degrees per second
    float gyro_y_dps;
    float gyro_z_dps;
    
    // Calculated tilt angles from accelerometer (in degrees)
    float roll;   // Rotation around X axis
    float pitch;  // Rotation around Y axis
};

extern daisy::I2CHandle i2c1_handle;

SensorData ReadSensorData();
void InitESP32();
bool TestESP32();
#endif // MPU6050_H
