#include "daisy_seed.h"
#include "daisysp.h"
#include "i2c.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hardware;
I2CHandle i2c1_handle;

int main() {
    hardware.Init();
    hardware.StartLog(true);
    hardware.Print("Initializing I2C...\n");

    I2CHandle::Config i2c1_conf;
    i2c1_conf.periph             = I2CHandle::Config::Peripheral::I2C_1;
    i2c1_conf.mode               = I2CHandle::Config::Mode::I2C_SLAVE;
    i2c1_conf.speed              = I2CHandle::Config::Speed::I2C_100KHZ;
    i2c1_conf.pin_config.scl     = seed::D13;
    i2c1_conf.pin_config.sda     = seed::D14;
    i2c1_conf.address            = 0x10;

    if (i2c1_handle.Init(i2c1_conf) != I2CHandle::Result::OK) {
        hardware.Print("I2C Init Failed!\n");
    } else {
        hardware.Print("I2C Init OK\n");
    }

    while (true) {
        SensorData data = ReadSensorData();
        hardware.PrintLine("=== Sensor Data ===");

        // Raw values
        hardware.PrintLine("Accel Raw  X: %d  Y: %d  Z: %d",
            data.accel_x, data.accel_y, data.accel_z);
        hardware.PrintLine("Gyro  Raw  X: %d  Y: %d  Z: %d",
            data.gyro_x, data.gyro_y, data.gyro_z);

        // Physical units
        hardware.PrintLine("Accel (g)  X: %.3f  Y: %.3f  Z: %.3f",
            data.accel_x_g, data.accel_y_g, data.accel_z_g);
        hardware.PrintLine("Gyro (dps) X: %.3f  Y: %.3f  Z: %.3f",
            data.gyro_x_dps, data.gyro_y_dps, data.gyro_z_dps);

        // Angles
        hardware.PrintLine("Roll:  %.3f deg", data.roll);
        hardware.PrintLine("Pitch: %.3f deg", data.pitch);

        hardware.PrintLine("===================");
    }
}