#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;

DaisySeed hw;
daisysp::Oscillator osc;

void MyCallback(AudioHandle::InputBuffer in,
                AudioHandle::OutputBuffer out,
                size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        // The oscillator's Process function synthesizes, and
        // returns the next sample.
        float sine_signal = osc.Process();
        out[0][i] = sine_signal;
        out[1][i] = sine_signal;
    }
}

int main(void)
{
    hw.Init();
    // We initialize the oscillator with the sample rate of the hardware
    // this ensures that the frequency of the Oscillator will be accurate.
    osc.Init(hw.AudioSampleRate());
    hw.StartAudio(MyCallback);
    while(1) {}
}