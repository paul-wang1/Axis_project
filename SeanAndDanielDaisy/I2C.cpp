#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hardware;

int main()
{
    // Initialize the Daisy Seed hardware (including LED)
    hardware.Init();

    // Configure the I2C handle
    I2CHandle::Config i2c1_conf;
    i2c1_conf.periph = I2CHandle::Config::Peripheral::I2C_1;
    i2c1_conf.mode = I2CHandle::Config::Mode::I2C_MASTER;
    i2c1_conf.speed = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c1_conf.pin_config.scl = seed::D11;
    i2c1_conf.pin_config.sda = seed::D12;

    // Initialize the I2C handle
    I2CHandle i2c1_handle;
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


    // Main loop

    // size of the buffer
    uint8_t buffer[16];
    while (true)
    {
        // Try to receive data -- got from 
        // https://daisy.audio/tutorials/_a9_Getting_Started-I2C/#blocking-transmit-and-receive
        I2CHandle::Result result = i2c1_handle.ReceiveBlocking(
            0x28,
            buffer,
            sizeof(buffer),
            1000
        );

        // Flash LED if data was received successfully
        if (result == I2CHandle::Result::OK)
        {
            hardware.SetLed(true);
            hardware.DelayMs(1000);
            hardware.SetLed(false);
            hardware.DelayMs(1000);
        }
    }
}