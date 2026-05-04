// ====== Config ======
#include <math.h>

// Optional: set to true to auto-calibrate micBias at startup
#define AUTO_CALIBRATE_BIAS true

// Optional CD4511 control pins; set to -1 if hard-wired in hardware
// Normal run requires: LE=LOW, BI=HIGH, LT=HIGH
const int CD4511_LE = -1;  // Latch Enable (LOW = transparent)
const int CD4511_BI = -1;  // Blanking (HIGH = enabled)
const int CD4511_LT = -1;  // Lamp Test (HIGH = normal)

// ====== Pin Configuration ======
const int micPin = 36;                    // ADC1_CH0 (input-only)
const int ledPins[9] = {19, 18, 5, 17, 16, 4, 0, 2, 15};

const int bcd1Pins[4] = {12, 14, 27, 26}; // Ones digit: LSB->MSB
const int bcd2Pins[4] = {3, 1, 22, 23};   // Tens digit: LSB->MSB

// ====== Microphone and Signal Settings ======
int micBias = 1700;                       // Will be overridden if auto-calibrated
const int sampleWindow = 100;             // Samples per RMS calculation
const float refAmplitude = 50.0f;         // Reference amplitude for 0 dB (tune)

// ====== Smoothing ======
float smoothedDb = 0.0f;
const float smoothing = 0.8f;             // 1=no smoothing, 0=very slow

// ====== Helpers ======
static inline void setIfValid(int pin, int level) {
  if (pin >= 0) { pinMode(pin, OUTPUT); digitalWrite(pin, level); }
}

static inline void writeBCD(int value, const int pins[4]) {
  value &= 0x0F;
  for (int j = 0; j < 4; j++) {
    digitalWrite(pins[j], (value >> j) & 0x01);
  }
}

static inline void setDisplays(int tens, int ones) {
  writeBCD(ones, bcd1Pins);
  writeBCD(tens, bcd2Pins);
}

static inline void setBar(int count) {
  if (count < 0) count = 0;
  if (count > 9) count = 9;
  for (int i = 0; i < 9; i++) {
    digitalWrite(ledPins[i], (i < count) ? HIGH : LOW);
  }
}

int calibrateMicBias(int samples = 800, int msBetween = 1) {
  long sum = 0;
  int minv = 4095, maxv = 0;
  for (int i = 0; i < samples; i++) {
    int v = analogRead(micPin);
    sum += v;
    if (v < minv) minv = v;
    if (v > maxv) maxv = v;
    delay(msBetween);
  }
  int avg = (int)(sum / samples);
  Serial.print("Calibrated micBias: "); Serial.println(avg);
  Serial.print("ADC min/max: "); Serial.print(minv); Serial.print("/"); Serial.println(maxv);
  return avg;
}

float getRMSAmplitude() {
  long sumOfSquares = 0;
  for (int i = 0; i < sampleWindow; i++) {
    int raw = analogRead(micPin);
    int sample = raw - micBias;
    sumOfSquares += (long)sample * (long)sample;
  }
  float meanSquare = (float)sumOfSquares / (float)sampleWindow;
  return sqrtf(meanSquare);
}

void setup() {
  Serial.begin(115200);

  // LEDs
  for (int i = 0; i < 9; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  // BCD outputs for both digits
  for (int i = 0; i < 4; i++) {
    pinMode(bcd1Pins[i], OUTPUT); digitalWrite(bcd1Pins[i], LOW);
    pinMode(bcd2Pins[i], OUTPUT); digitalWrite(bcd2Pins[i], LOW);
  }

  // ESP32 ADC configuration
  analogReadResolution(12);            // 0–4095
  analogSetAttenuation(ADC_11db);      // ~3.6 V full-scale (module dependent)

  // CD4511 control defaults: LE=LOW, BI=HIGH, LT=HIGH
  setIfValid(CD4511_LE, LOW);
  setIfValid(CD4511_BI, HIGH);
  setIfValid(CD4511_LT, HIGH);

  // Optional: auto-calibrate micBias at startup
  if (AUTO_CALIBRATE_BIAS) {
    delay(1000); // allow settling
    Serial.println("Calibrating micBias... Keep quiet");
    micBias = calibrateMicBias(800, 1);
  }

  // Sanity: start at "00" and bar off
  setDisplays(0, 0);
  setBar(0);
}

void loop() {
  // 1) Measure RMS amplitude
  float rms = getRMSAmplitude();

  // 2) Convert to decibels (relative) with guard
  float decibels = 0.0f;
  if (rms > 1e-6f) {
    decibels = 20.0f * log10f(rms / refAmplitude);
  }
  if (decibels < 0.0f) decibels = 0.0f;
  if (decibels > 90.0f) decibels = 90.0f;

  // 3) Smooth
  smoothedDb = (decibels * smoothing) + (smoothedDb * (1.0f - smoothing));

  // 4) Map to LED count (input 6–22 -> output 0–9)
  const float inMin = 6.0f, inMax = 22.0f;
  const float outMin = 0.0f, outMax = 9.0f;
  float scaled = (smoothedDb - inMin) / (inMax - inMin) * (outMax - outMin) + outMin;
  int numLEDs = (int)(scaled + 0.5f);
  if (numLEDs < 0) numLEDs = 0;
  if (numLEDs > 9) numLEDs = 9;

  // 5) Update LED bar
  setBar(numLEDs);

  // 6) Show integer dB on 7-seg via CD4511
  int displayDb = (int)(smoothedDb + 0.5f); // rounded integer dB
  if (displayDb < 0) displayDb = 0;
  if (displayDb > 99) displayDb = 99;
  int ones = displayDb % 10;
  int tens = displayDb / 10;
  setDisplays(tens, ones);

  // 7) Debug
  Serial.print("RMS: "); Serial.print(rms);
  Serial.print("  dB: "); Serial.println(smoothedDb);

  // Small pause for smooth output and readable updates
  delay(10);
}