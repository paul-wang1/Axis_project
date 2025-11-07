#include "DaisyDuino.h"
#include <cstdlib>              // for rand()
using namespace daisysp;
DaisyHardware hw;
size_t num_channels;
static Metro      metroClk;     // <-- renamed to avoid clash with time.h::clock()
static Oscillator osc;          // sine osc
void AudioCallback(float **in, float **out, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (metroClk.Process()) {
      // pick a new random freq (110..610 Hz)
      float f = 110.0f + (rand() % 501);
      osc.SetFreq(f);
    }
    float s = osc.Process();    // one sample
    for (size_t ch = 0; ch < num_channels; ch++) {
      out[ch][i] = s;           // same signal on all outputs
    }
  }
}
void setup() {
  // Init Seed @ 48 kHz
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  num_channels = hw.num_channels;
  float sr = DAISY.get_samplerate();
  // 2 Hz tick; sine setup
  metroClk.Init(2.0f, sr);
  osc.Init(sr);
  osc.SetWaveform(Oscillator::WAVE_SIN);
  osc.SetFreq(220.0f);
  osc.SetAmp(0.25f);
  DAISY.begin(AudioCallback);
  Serial.begin(115200);
  delay(300);
  Serial.println("Seed Metro Sine ready (pins 18=L, 19=R, 20=AGND)");
}
void loop() {}