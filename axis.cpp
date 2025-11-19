#include "mpu6050.h"
#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;


DaisySeed hw;
Oscillator osc;
SensorData data;
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{

    static uint32_t counter = 0;

    
    // Read orientation around every 100ms
    if (counter++ % 1200 == 0) {
        data = ReadSensorData();
        // Pregain = 1.2; // Set this constant to change how loud it is

        // Map heading (0-360) to Gain
        // Gain    = (read.euler_heading / 360.0f) * 100 + 1.2;
        // // I'm not sure what dry wet here is but its something
        // drywet  = (read.euler_pitch + 180.0f) / 360.0f;

        // // Set bypass to Orientation of device
        // bypassSoft = read.euler_roll < 0;
        // bypassHard = read.euler_roll >= 0;
        osc.SetFreq(440 + data.pitch/3.6);
    }

    // Pass-through audio for now
    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = osc.Process();
        out[1][i] = osc.Process();
    }
}

int main(void)
{
    // Initialize Daisy
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    
    // Start USB serial for debugging
    hw.StartLog(false);
    System::Delay(1000);
    
    hw.PrintLine("=== MPU6050 Test ===");
    
    // Initialize MPU6050
    InitMPU6050();
    hw.PrintLine("MPU6050 Initialized");
    
    // Test connection
    if(TestMPU6050()) {
        hw.PrintLine("MPU6050 Connected!");
    } else {
        hw.PrintLine("ERROR: MPU6050 Not Found!");
        while(1) {
            hw.SetLed(true);
            System::Delay(100);
            hw.SetLed(false);
            System::Delay(100);
        }
    }
    float sample_rate = hw.AudioSampleRate();
    osc.Init(sample_rate);
    osc.SetWaveform(Oscillator::WAVE_SIN);
    osc.SetAmp(0.5f);
    osc.SetFreq(440.0f);
    
    hw.StartAudio(AudioCallback);
    
    // Start audio
    // hw.StartAudio(AudioCallback);
    hw.PrintLine("Starting main loop...\n");
    
    // Main loop - read sensor data
    // uint32_t last_read = System::GetNow();
    
    while(1)
    {
        // Read sensor every 100ms
        // if(System::GetNow() - last_read >= 100)
        // {
        //     last_read = System::GetNow();
            
        //     // Read sensor data
        //     SensorData data = ReadSensorData();
            
        //     // Print accelerometer (in g's)
        //     hw.PrintLine("Accel: X=%.2f Y=%.2f Z=%.2f g", 
        //                  data.accel_x_g, data.accel_y_g, data.accel_z_g);
            
        //     // Print gyroscope (in degrees/second)
        //     hw.PrintLine("Gyro:  X=%.1f Y=%.1f Z=%.1f dps", 
        //                  data.gyro_x_dps, data.gyro_y_dps, data.gyro_z_dps);
            
        //     // Print calculated angles (in degrees)
        //     hw.PrintLine("Angles: Roll=%.1f Pitch=%.1f\n", 
        //                  data.roll, data.pitch);
            
        //     // Toggle LED to show it's working
        //     hw.SetLed(!hw.Led());
            
        //     // You can use these values to control audio effects
        //     // For example:
        //     // float tilt = data.roll;  // -180 to +180 degrees
        //     // Use tilt to control an effect parameter
        // }
    }
}