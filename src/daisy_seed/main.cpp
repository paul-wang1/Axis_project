#include "daisy_seed.h"
#include "daisysp.h"
#include <math.h>
#include "i2c.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hardware;


volatile float trem_depth = 0.5f;
volatile float trem_freq = 5.0f;

const float GYRO_SENSITIVITY = 131.0f;   // LSB per °/s for ±250°/s range
const float ACCEL_SENSITIVITY = 16384.0f; // LSB per g for ±2g range

I2CHandle i2c1_handle;
uint8_t buffer[14];


/* 
 * Read in Sensor data from the ESP32
 * @effects: Will block until sensor 14 bytes are read brom the sensor
 */
SensorData ReadSensorData() {
    // Try to receive data
        I2CHandle::Result result = i2c1_handle.ReceiveBlocking(
            0x28,
            buffer,
            sizeof(buffer),
            1000
        );

        // Flash LED if data was received successfully
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
            SensorData data;
            return data;
        }
}


void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // tremolo.SetDepth(trem_depth);
    // tremolo.SetFreq(trem_freq);

    // for (size_t i = 0; i < size; i++) {
    //     float dry = in[0][i];
    //     float wet = tremolo.Process(dry);
    //     out[0][i] = wet;
    //     out[1][i] = wet;
    // }
    // data = ReadSensorData();
    // hw.PrintLine("Pitch: %d", data.pitch);

    for (size_t i = 0; i < size; i++) {
        out[0][i] = in[0][i] * 1;
        out[1][i] = in[1][i] * 1;
    }
}


int main(){
    // Initialize the Daisy Seed hardware (including LED)
    hardware.Init();
    hardware.StartLog(true);

    // Configure the I2C handle
    I2CHandle::Config i2c1_conf;
    i2c1_conf.periph = I2CHandle::Config::Peripheral::I2C_1;
    i2c1_conf.mode = I2CHandle::Config::Mode::I2C_MASTER;
    i2c1_conf.speed = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c1_conf.pin_config.scl = seed::D11;
    i2c1_conf.pin_config.sda = seed::D12;

    // Initialize the I2C handle
    if (i2c1_handle.Init(i2c1_conf) != I2CHandle::Result::OK)
    {
        // Init failed - rapid blink to indicate error
        while (true)
        {
            hardware.SetLed(true);
            hardware.DelayMs(100);
            hardware.SetLed(false);
            hardware.DelayMs(100);
        }
    }

    uint8_t buffer[14];

    while (true)
    {
        SensorData data = ReadSensorData();
        for(int i = 0; i < 14; i++) {
            hardware.Print("%x", buffer[i]);
        }
        hardware.Print("\n");
    }
}