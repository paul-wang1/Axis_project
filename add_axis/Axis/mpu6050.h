#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

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

SensorData ReadSensorData();
void InitMPU6050();
bool TestMPU6050();
#endif // MPU6050_H