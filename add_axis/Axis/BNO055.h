#include "daisy_seed.h"
#include "daisysp.h"

#ifndef BNO055
#define BNO055

// Orientation data
typedef struct Angles {
    float euler_heading;
    float euler_roll;
    float euler_pitch;
} Angles;


void WriteBNO055Register(uint8_t reg, uint8_t value);
void ReadBNO055Register(uint8_t reg, uint8_t arr[], uint8_t size);
void InitBNO055();
Angles ReadEulerAngles();
#endif