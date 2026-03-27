#include "i2c.h"
#include "daisy_seed.h"
#include "daisysp.h"
#include <math.h>

using namespace daisy;
using namespace daisysp;


const float GYRO_SENSITIVITY = 131.0f;   // LSB per °/s for ±250°/s range
const float ACCEL_SENSITIVITY = 16384.0f; // LSB per g for ±2g range

SensorData ReadSensorData() {
    // Try to receive data
    uint8_t buffer[16];
    I2CHandle::Result result = i2c1_handle.ReceiveBlocking(
        0x00,
        buffer,
        sizeof(buffer),
        1000
    );

    // Parse Data if the I2C data was sent successfully
    if (result == I2CHandle::Result::OK)
    {
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
    } else {
        // If the data isn't parsed correctly return an empty struct
        SensorData data;
        return data;
    }
}