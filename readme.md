# Vital Monitor

## Real-Time Cardiovascular and Stress Monitoring Using PPG Signal Processing

A standalone embedded biomedical analytics system that performs real-time Heart Rate (BPM), Blood Oxygen Saturation (SpO₂), and Heart Rate Variability (HRV) monitoring using the MAX30102 optical sensor and ESP32 microcontroller.

The system processes raw Photoplethysmography (PPG) signals directly on the ESP32 without requiring cloud services, smartphones, or external computers. All vital parameters are displayed on an OLED screen, while an emergency push-button provides immediate alert functionality.

---

# Overview

Smart Vital Monitor is an embedded healthcare device designed to provide continuous monitoring of physiological parameters using optical sensing technology.

Unlike traditional pulse oximeters that only display BPM and SpO₂, this system performs additional Heart Rate Variability (HRV) analysis to estimate stress and fatigue levels directly on the microcontroller.

The entire signal processing pipeline runs locally on the ESP32, enabling real-time monitoring without internet connectivity.

---

# Features

## Physiological Monitoring

* Heart Rate (BPM)
* Blood Oxygen Saturation (SpO₂)
* Heart Rate Variability (HRV)
* Stress/Fatigue Detection

## Embedded Analytics

* Raw PPG Signal Acquisition
* Digital Signal Processing (DSP)
* Bandpass Filtering
* Peak Detection
* RR Interval Analysis
* HRV Computation

## User Interface

* SSD1306 OLED Display
* Real-Time Monitoring
* Emergency Alert Screen

## Standalone Operation

* No Mobile App Required
* No Cloud Dependency
* No Internet Connection Needed

---

# Hardware Components

| Component                        | Quantity |
| -------------------------------- | -------- |
| ESP32 DevKit V1 / ESP32-WROOM-32 | 1        |
| MAX30102 Sensor                  | 1        |
| SSD1306 OLED Display (128x64)    | 1        |
| Push Button                      | 1        |
| Buzzer                           | 1        |
| wires                            | 15       |


---

# System Architecture

```text
Finger
   ↓
MAX30102
   ↓
Raw PPG Signal
   ↓
Digital Signal Processing
   ↓
Peak Detection
   ↓
RR Interval Extraction
   ↓
Feature Extraction
(BPM / SpO₂ / HRV)
   ↓
Health Classification
   ↓
OLED Display
```

---

# Hardware Connections

## MAX30102 Connections

| MAX30102 | ESP32   |
| -------- | ------- |
| VIN      | 3.3V    |
| GND      | GND     |
| SDA      | GPIO 21 |
| SCL      | GPIO 22 |
| INT      | GPIO 19 |

---

## OLED SSD1306 Connections

| OLED | ESP32   |
| ---- | ------- |
| VCC  | 3.3V    |
| GND  | GND     |
| SDA  | GPIO 21 |
| SCL  | GPIO 22 |

---

## Emergency Button

| Button     | ESP32   |
| ---------- | ------- |
| One Side   | GPIO 18 |
| Other Side | GND     |


Configuration:

```cpp
pinMode(18, INPUT_PULLUP);
```
---

## Alert Buzzer

| Buzzer     | ESP32   |
| ---------- | ------- |
| One Side   | GPIO 4  |
| Other Side | GND     |

---

# Signal Processing Pipeline

## Step 1: Data Acquisition

The MAX30102 continuously captures:

* Infrared Signal (IR)
* Red Signal

Sampling Rate:

```text
100 Samples/Second
```

---

## Step 2: Bandpass Filtering

Filter Range:

```text
0.7 Hz – 4 Hz
```

Purpose:

* Remove baseline drift
* Reduce ambient noise
* Suppress motion artifacts

---

## Step 3: Peak Detection

The filtered PPG waveform is analyzed to identify heartbeat peaks.

Output:

```text
RR Intervals
```

---

## Step 4: Heart Rate Calculation

Formula:

```text
Heart Rate = 60 / Mean RR Interval
```

---

## Step 5: SpO₂ Estimation

Ratio Calculation:

```text
R = (ACred / DCred) / (ACir / DCir)
```

Approximation:

```text
SpO₂ ≈ 104 − 17R
```

---

## Step 6: HRV Analysis

Metrics:

* RMSSD
* SDNN

Purpose:

* Stress Monitoring
* Fatigue Detection
* Autonomic Nervous System Analysis

---

# Health State Classification

| Condition                | State      |
| ------------------------ | ---------- |
| RMSSD > 40 ms            | NORMAL     |
| RMSSD 20–40 ms           | STRESS     |
| RMSSD < 20 ms            | FATIGUE    |
| SpO₂ < 92%               | LOW OXYGEN |
| Emergency Button Pressed | EMERGENCY  |

---

# OLED Display Example

```text
------------------------
HR   : 78 BPM
SpO2 : 97 %

HRV  : 45 ms
STATE: NORMAL

[READY]
------------------------
```

Emergency Screen:

```text
------------------------
!!! EMERGENCY !!!
CALL FOR HELP
------------------------
```

---

# Development Tools

## Hardware

* ESP32 DevKit V1
* MAX30102
* SSD1306 OLED
* Buzzer
* LED

## Software

* Arduino IDE
* Wire Library
* Adafruit SSD1306 Library
* MAX30102 Library

---

# Applications

## Healthcare

* Rural Clinics
* Health Screening
* Community Medical Camps

## Research

* Biomedical Signal Processing
* HRV Analysis
* PPG Research

## Safety Systems

* Driver Fatigue Detection
* Industrial Worker Monitoring
* Emergency Monitoring

## Fitness

* Recovery Analysis
* Stress Monitoring
* Training Optimization

---

# Results

Expected Performance:

| Parameter          | Accuracy     |
| ------------------ | ------------ |
| Heart Rate         | ±2 BPM       |
| SpO₂               | ±2–3%        |
| HRV                | Experimental |
| Emergency Response | < 5 ms       |

---

# Limitations

* Sensitive to finger movement
* Not a certified medical device
* Requires proper finger placement
* No long-term storage
* No remote monitoring

---

# Future Enhancements

* Bluetooth Connectivity
* Wi-Fi Dashboard
* TinyML Stress Classification
* SD Card Logging
* Battery-Powered Wearable Version
* Motion Artifact Rejection
* ECG Integration Using AD8232
* Remote Patient Monitoring

---

# Disclaimer

This project is intended for educational, research, and prototyping purposes only. It is not approved for medical diagnosis or treatment and should not be used as a replacement for professional healthcare equipment.

---

# License

MIT License
