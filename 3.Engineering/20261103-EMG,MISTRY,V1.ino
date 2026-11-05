
#include <math.h>

const int NumSamples = 1000;
const float fs = 2000.0f;

const float lowCutoff = 20.0f;
const float highCutoff = 450.0f;

float RawVoltage[NumSamples];
float FilteredVoltage[NumSamples];
unsigned long Time[NumSamples];

struct Biquad {
  float b0, b1, b2, a1, a2;
  float x1 = 0, x2 = 0;
  float y1 = 0, y2 = 0;

  float filter(float x) {
    float y = b0*x + b1*x1 + b2*x2
            - a1*y1 - a2*y2;

    x2 = x1;
    x1 = x;
    y2 = y1;
    y1 = y;

    return y;
  }
};

Biquad highpass;
Biquad lowpass;

void configureButterworth(Biquad &f, float fc,
                          float fs, bool hp) {

  const float Q = 0.70710678f;
  float w0 = 2.0f * PI * fc / fs;
  float c = cosf(w0);
  float alpha = sinf(w0) / (2.0f * Q);
  float a0 = 1.0f + alpha;

  if (hp) {
    f.b0 = (1.0f + c) / (2.0f * a0);
    f.b1 = -(1.0f + c) / a0;
    f.b2 = f.b0;
  } else {
    f.b0 = (1.0f - c) / (2.0f * a0);
    f.b1 = (1.0f - c) / a0;
    f.b2 = f.b0;
  }

  f.a1 = -2.0f * c / a0;
  f.a2 = (1.0f - alpha) / a0;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  configureButterworth(highpass, lowCutoff, fs, true);
  configureButterworth(lowpass, highCutoff, fs, false);

  const unsigned long samplePeriod = 500; // us
  unsigned long startTime = micros();
  unsigned long nextSample = startTime;

  for (int i = 0; i < NumSamples; i++) {

    while ((int32_t)(micros() - nextSample) < 0) {
      // Wait for next sample
    }

    unsigned long timestamp = micros();
    nextSample += samplePeriod;

    int adc = analogRead(A0);

    float raw = adc * (3.3f / 4095.0f);

    // Remove DC offset with high-pass filter
    float filtered = highpass.filter(raw);

    // Remove high-frequency noise
    filtered = lowpass.filter(filtered);

    RawVoltage[i] = raw;
    FilteredVoltage[i] = filtered;
    Time[i] = timestamp - startTime;
  }

  Serial.println("Time_s,Raw_V,Filtered_V");

  for (int i = 0; i < NumSamples; i++) {
    Serial.print(Time[i] / 1000000.0f, 6);
    Serial.print(",");
    Serial.print(RawVoltage[i], 5);
    Serial.print(",");
    Serial.println(FilteredVoltage[i], 5);
  }
}

void loop() {
}
