#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hardware;

int main()
{
    bool led_state;
    led_state = true;
    
    // Initialize the Daisy Seed hardware (including LED)
    hardware.Init();
    hardware.Configure();

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
            // Set the onboard LED
            hardware.SetLed(led_state);

            // Toggle the LED state for the next time around.
            led_state = !led_state;

            // Wait 500ms
            System::Delay(500);
        }
    }

    uint8_t buffer[16];

    // Main loop
    while (true)
    {
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
            hardware.SetLed(true);
            hardware.DelayMs(200);
            hardware.SetLed(false);
            hardware.DelayMs(200);
        }
    }
}