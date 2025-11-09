#include "BNO055.h"
#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

I2CHandle  i2c;

// BNO055 I2C Base Address
const uint8_t BNO055_ADDRESS = 0x28;

// BNO055 Register Addresses
const uint8_t BNO055_OPR_MODE_ADDR = 0x3D;
const uint8_t BNO055_PWR_MODE_ADDR = 0x3E;
const uint8_t BNO055_EULER_H_LSB_ADDR = 0x1A;
const uint8_t BNO055_PAGE_ID_ADDR = 0x07;

// Operation modes
const uint8_t OPERATION_MODE_NDOF = 0x0C;
const uint8_t POWER_MODE_NORMAL = 0x00;


void WriteBNO055Register(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    i2c.TransmitBlocking(BNO055_ADDRESS, buffer, 2, 100);
}

void ReadBNO055Register(uint8_t reg, uint8_t arr[], uint8_t size) {
    i2c.TransmitBlocking(BNO055_ADDRESS, &reg, 1, 100);
    i2c.ReceiveBlocking(BNO055_ADDRESS, arr, size, 100);
}

void InitBNO055() {
    // Configure I2C for BNO055
    I2CHandle::Config i2c_conf;
    i2c_conf.periph = I2CHandle::Config::Peripheral::I2C_1;
    i2c_conf.mode   = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_conf.speed  = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_conf.pin_config.scl = seed::D11;
    i2c_conf.pin_config.sda = seed::D12;
    
    // Initialize I2C
    if (i2c.Init(i2c_conf) != I2CHandle::Result::OK) {
        // I2C initialization failed - handle error
        while(1);
    }
    
    // Initialize BNO055
    System::Delay(100); // Give sensor time to boot
    // Set to config mode first
    WriteBNO055Register(BNO055_OPR_MODE_ADDR, 0x00);
    System::Delay(30);
    
    // Set to normal power mode
    WriteBNO055Register(BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);
    System::Delay(10);
    
    // Select page 0
    WriteBNO055Register(BNO055_PAGE_ID_ADDR, 0x00);
    System::Delay(10);
    
    // Set to NDOF mode (fusion mode with magnetometer)
    WriteBNO055Register(BNO055_OPR_MODE_ADDR, OPERATION_MODE_NDOF);
    System::Delay(20);
}

Angles ReadEulerAngles() {
    uint8_t buffer[6];
    uint8_t reg = BNO055_EULER_H_LSB_ADDR;
    
    // Read 6 bytes starting from Euler heading LSB
    ReadBNO055Register(reg, buffer, 6);
    
    // Separate Angles
    int16_t heading_raw = (buffer[1] << 8) | buffer[0];
    int16_t roll_raw = (buffer[3] << 8) | buffer[2];
    int16_t pitch_raw = (buffer[5] << 8) | buffer[4];
    
    Angles read;
    // Convert to degrees
    read.euler_heading = heading_raw / 16.0f;
    read.euler_roll = roll_raw / 16.0f;
    read.euler_pitch = pitch_raw / 16.0f;

    return read;
}