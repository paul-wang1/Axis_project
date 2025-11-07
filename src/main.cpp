#include "DaisyDuino.h"

DaisyHardware hw;

void MyCallback(float **in, float **out, size_t size) 
{
    for (size_t i = 0; i < size; i++) 
    {
        // Pass input directly to output
        out[0][i] = in[0][i];  // Left channel
        out[1][i] = in[1][i];  // Right channel
    }
}

void setup() 
{
    // Initialize Daisy Seed with 48kHz sample rate
    hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
    
    // Start audio processing
    DAISY.begin(MyCallback);
}

void loop() 
{
    // Audio runs automatically in the callback
}

