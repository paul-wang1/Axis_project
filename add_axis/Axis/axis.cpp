// #include "mpu6050.h"
// #include "daisy_seed.h"
// #include "daisysp.h"

// using namespace daisy;
// using namespace daisysp;


// DaisySeed hw;
// Oscillator osc;
// PitchShifter ps;
// SensorData data;
// Tremolo tremolo;


// volatile float trem_depth = 0.5f;
// volatile float trem_freq = 5.0f;
// void AudioCallback(AudioHandle::InputBuffer  in,
//                    AudioHandle::OutputBuffer out,
//                    size_t                    size)
// {


// //          tremolo.SetDepth(trem_depth);
// //     tremolo.SetFreq(trem_freq);

// // for (size_t i = 0; i < size; i++) {
// //         float dry = in[0][i];
// //         float wet = tremolo.Process(dry);
        
// //         out[0][i] = wet;
// //         out[1][i] = wet;
// //     }

//     for (size_t i = 0; i < size; i++) {
//         out[0][i] = in[0][i];
//         out[1][i] = in[1][i];
//     }
// }


// float Map(float x, float in_min, float in_max, float out_min, float out_max) {
//     x = fminf(fmaxf(x, in_min), in_max);  // Clamp input
//     return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
// }


// int main(void)
// {
//     // Initialize Daisy
//     hw.Configure();
//     hw.Init();
//     hw.SetAudioBlockSize(4);
    
//     // Start USB serial for debugging
//     hw.StartLog(true);
//     System::Delay(1000);
    
//     hw.PrintLine("=== MPU6050 Test ===");
    
//     // Initialize MPU6050
//     InitMPU6050();
//     hw.PrintLine("MPU6050 Initialized");
    
//     // Test connection
//     if(TestMPU6050()) {
//         hw.PrintLine("MPU6050 Connected!");
//     } else {
//         hw.PrintLine("ERROR: MPU6050 Not Found!");
//         while(1) {
//             hw.SetLed(true);
//             System::Delay(100);
//             hw.SetLed(false);
//             System::Delay(100);
//         }
//     }
//     tremolo.Init(hw.AudioSampleRate());
//     tremolo.SetWaveform(Oscillator::WAVE_SIN);
    
//     hw.StartAudio(AudioCallback);

//     int sample_rate = hw.AudioSampleRate();

//     // ps.Init(sample_rate);

    
//     // Start audio
//     // hw.StartAudio(AudioCallback);
//     hw.PrintLine("Starting main loop...\n");
// ///
//     // Main loop - read sensor data
//     uint32_t last_read = System::GetNow();
    
//     while(1)
//     {
//         // Read sensor every 100ms
//         if(System::GetNow() - last_read >= 100)
//         {
//             // hw.PrintLine("Reading Data now...\n");
//             data = ReadSensorData();
//             last_read = System::GetNow();

//             // ps.SetTransposition((int(data.pitch/ 10) % 36));



//             trem_depth = Map(data.pitch, -45.0f, 45.0f, 0.0f, 1.0f);
        
//         // Example: roll controls speed
//              trem_freq = Map(data.roll, -45.0f, 45.0f, 2.0f, 12.0f);

//         }
//     }
// }

// #include "mpu6050.h"
// #include "daisy_seed.h"
// #include "daisysp.h"

// using namespace daisy;
// using namespace daisysp;


// DaisySeed hw;
// Oscillator osc;

// void AudioCallback(AudioHandle::InputBuffer  in,
//                    AudioHandle::OutputBuffer out,
//                    size_t                    size)
// {
//     //osc.SetFreq(440 + data.pitch/3.6);
//     // Pass-through audio for now
//     for(size_t i = 0; i < size; i++)
//     {
//         out[0][i] = osc.Process();
//         out[1][i] = osc.Process();
//     }
// }

// int main(void)
// {
//     // Initialize Daisy
//     hw.Configure();
//     hw.Init();
//     hw.SetAudioBlockSize(4);
    
//     // Start USB serial for debugging
//     // hw.StartLog(false);
//     // System::Delay(1000);
    
//     float sample_rate = hw.AudioSampleRate();
//     osc.Init(sample_rate);
//     osc.SetWaveform(Oscillator::WAVE_SIN);
//     osc.SetAmp(0.5f);
//     osc.SetFreq(440.0f);
    
//     //Start audio
//     hw.StartAudio(AudioCallback);
    
//     while(1)
//     {

//     }
// }


            // int data_sample = 10;
            // hw.PrintLine("Sample data:%d", 10);
            // Print accelerometer (in g's)
            // hw.PrintLine("Accel: X=%.2f Y=%.2f Z=%.2f g", 
            //              data.accel_x_g, data.accel_y_g, data.accel_z_g);
            
            // Print gyroscope (in degrees/second)
            // hw.PrintLine("Gyro:  X=%.1f Y=%.1f Z=%.1f dps", 
            //              data.gyro_x_dps, data.gyro_y_dps, data.gyro_z_dps);
            // int tilt = int(data.roll);  // -180 to +180 degrees
            // Print calculated angles (in degrees)
            // hw.PrintLine("Angles: Roll= %d", tilt);


#include "mpu6050.h"
#include "daisy_seed.h"
#include "daisysp.h"

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

float Map(float x, float in_min, float in_max, float out_min, float out_max) {
    x = fminf(fmaxf(x, in_min), in_max);
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int main(void)
{
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    
    // No StartLog - it might interfere
    
    InitMPU6050();
    
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