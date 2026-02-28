#include "daisy_seed.h"
#include "daisysp.h"
#include "i2c.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;
Tremolo tremolo;
SensorData data;


volatile float trem_depth = 0.5f;
volatile float trem_freq = 5.0f;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    tremolo.SetDepth(trem_depth);
    tremolo.SetFreq(trem_freq);

    for (size_t i = 0; i < size; i++) {
        float dry = in[0][i];
        float wet = tremolo.Process(dry);
        out[0][i] = wet;
        out[1][i] = wet;
    }
}

int main()
{

    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    
    InitESP32();
    
    float sample_rate = hw.AudioSampleRate();
    tremolo.Init(sample_rate);
    tremolo.SetWaveform(Oscillator::WAVE_SIN);
    
    hw.StartAudio(AudioCallback);
    
    uint32_t last_read = System::GetNow();
    
    while(1)
    {
        if(System::GetNow() - last_read >= 100)
        {
            data = ReadSensorData();
            last_read = System::GetNow();
            
            trem_depth = Map(data.pitch, -45.0f, 45.0f, 0.0f, 1.0f);
            trem_freq = Map(data.roll, -45.0f, 45.0f, 2.0f, 12.0f);
        }
    }
}
