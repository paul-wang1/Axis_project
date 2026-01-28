#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hardware;
I2CHandle i2c;

int main()
{
    // Configure the handle
    I2CHandle::Config i2c1_conf;
    i2c1_conf.periph = I2CHandle::Config::Peripheral::I2C_1;
    i2c1_conf.mode = I2CHandle::Config::Mode::I2C_MASTER; // Use the peripheral as a controller
    i2c1_conf.speed = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c1_conf.pin_config.scl = seed::D11; // Must match pinout for I2C1 SCL
    i2c1_conf.pin_config.sda = seed::D12; // Must match pinout for I2C1 SDA
    // Address not needed for controller!

    // Initialise the handle
    I2CHandle i2c1_handle;
    if (i2c1_handle.Init(i2c1_conf) != I2CHandle::Result::OK)
    {
        // Something went wrong! Handle it here.
    }

    uint8_t buffer[4] = {0, 1, 2, 3}; // Data to send
    i2c1_handle.TransmitBlocking(
        0x28,                               // Target address
        buffer,                             // Pointer to buffer
        sizeof(buffer) / sizeof(buffer[0]), // Number of bytes to send
        1000                                // Try for this many milliseconds before failing
    );
}