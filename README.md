# ESP32 Sound Level Meter (VU Meter)

## Overview
This project implements a real-time sound level meter using an ESP32 and a microphone module. It measures ambient sound, converts it into a decibel-like value, and displays the output in two ways:
- Numeric display (00–99 dB) using 2× 7-segment displays via CD4511
- Visual LED bar (9 LEDs) representing sound intensity

This was made as part of Electrivire - Secure The Sound, a competition organised by Electroholics IIITD.

---

## Features
- Real-time sound measurement
- RMS-based signal processing
- Logarithmic decibel conversion
- Smooth output (anti-flicker)
- Auto microphone bias calibration
- Dual display output (numeric + bar graph)

---

##  Steps for logic

### 1. Microphone Input
The ESP32 reads analog signals from a microphone. Since the signal contains a DC offset, a bias value is calculated and subtracted to center the waveform.

### 2. RMS Calculation
100 samples are collected and processed using RMS (Root Mean Square) to determine signal strength.

Formula:
dB = 20 × log10(RMS / reference)

### 3. Decibel Conversion
The RMS value is converted into a logarithmic decibel scale and clamped between 0 and 90 dB.

### 4. Smoothing
A smoothing filter is applied:
smoothed = new × factor + old × (1 − factor)
This reduces flickering and stabilizes the display.

### 5. LED Bar Display
The decibel value is mapped to a range of 0–9 LEDs. More LEDs indicate higher sound levels.

### 6. 7-Segment Display
The decibel value is split into tens and ones digits and sent as BCD signals to CD4511 drivers, which control the displays.

---

## Hardware Connections

### Microphone Module
- VCC → 3.3V
- GND → GND
- OUT → GPIO 36 (ADC input)

---

### 💡 LED Bar (9 LEDs)
Each LED connection:
GPIO → 220Ω resistor → LED → GND

Pins used:
19, 18, 5, 17, 16, 4, 0, 2, 15

---

### 7-Segment Displays (via CD4511)

#### Ones Digit
- A → GPIO 12
- B → GPIO 14
- C → GPIO 27
- D → GPIO 26

#### Tens Digit
- A → GPIO 3
- B → GPIO 1
- C → GPIO 22
- D → GPIO 23

---

### ⚙️ CD4511 Control Pins
- LE (Latch Enable) → LOW
- BI (Blanking) → HIGH
- LT (Lamp Test) → HIGH

---

###  7-Segment Display Notes
- Use common cathode displays
- Each segment requires a 220Ω resistor

---

## 📊 System Flow
Microphone → ADC → RMS → dB Conversion → Smoothing → LED Bar + 7-Segment Display

---

## Project Summary
This project demonstrates a practical implementation of a sound level meter using ESP32. It processes real-time audio signals and provides both visual and numeric feedback, making it useful for noise monitoring and embedded system learning.

---

## Requirements
- ESP32 board
- Microphone module
- 2× CD4511 IC
- 2× 7-segment displays (common cathode)
- 9 LEDs
- 220Ω resistors

---

## Summary
It reads audio input from a microphone (analog pin), calculates the RMS amplitude, and converts it into a decibel (dB) value. The dB value is smoothed to avoid flickering. A 9-LED bar graph displays the sound intensity visually. Two 7-segment displays (via CD4511 BCD drivers) show the numeric dB value (00–99).
It can auto-calibrate the microphone bias at startup for better accuracy.
In short, it measures ambient sound level and shows it both as a number and as a LED bar indicator.

---