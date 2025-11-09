#include "daisy_seed.h"
#include "daisysp.h"
#include "BNO055.h"

using namespace daisy;
using namespace daisysp;

DaisySeed  hardware;
Oscillator osc;

bool bypassHard, bypassSoft;
float Pregain, Gain, drywet;

float hardClip(float in) {
    in = in > 1.f ? 1.f : in;
    in = in < -1.f ? -1.f : in;
    return in;
}

float softClip(float in) {
    if(in > 0)
        return 1 - expf(-in);
    return -1 + expf(in);
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                   size) {
    static uint32_t counter = 0;

    
    // Read orientation around every 100ms
    if (counter++ % 1200 == 0) {
        Angles read = ReadEulerAngles();
        Pregain = 1.2; // Set this constant to change how loud it is

        // Map heading (0-360) to Gain
        Gain    = (read.euler_heading / 360.0f) * 100 + 1.2;
        // I'm not sure what dry wet here is but its something
        drywet  = (read.euler_pitch + 180.0f) / 360.0f;

        // Set bypass to Orientation of device
        bypassSoft = read.euler_roll < 0;
        bypassHard = read.euler_roll >= 0;
        
    }
    
    // Fill the block with samples - continuous playback
    for(size_t i = 0; i < size; i += 2)
    {
        for(int chn = 0; chn < 2; chn++)
        {
            float input = osc.Process() * Pregain;
            float wet   = input;

            if(!bypassSoft || !bypassHard)
            {
                wet *= Gain;
            }

            if(!bypassSoft)
            {
                wet = softClip(wet);
            }

            if(!bypassHard)
            {
                wet = hardClip(wet);
            }

            out[chn][i] = wet * drywet + input * (1 - drywet);
        }
    }
}

int main(void) {
    hardware.Configure();
    hardware.Init();
    hardware.SetAudioBlockSize(4);
    
    float samplerate = hardware.AudioSampleRate();

    InitBNO055();

    // For Testing Purposes
    osc.Init(samplerate);
    osc.SetWaveform(Oscillator::WAVE_SIN);
    osc.SetAmp(0.5f);
    osc.SetFreq(440.0f);
    
    hardware.StartAudio(AudioCallback);


    
    for(;;) {}
}