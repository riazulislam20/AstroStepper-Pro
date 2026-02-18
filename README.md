🌌 AstroStepper Pro
Observatory-Grade 32-bit DDS Stepper Engine for ESP32

Release: v1.0

🚀 Overview

AstroStepper Pro is a high-precision stepper motor control library designed specifically for:

🌠 GoTo telescope mounts

📷 Astrophotography tracking systems

🛰 Satellite / comet tracking

🔭 High precision equatorial mounts

It replaces traditional interval-based stepping (like AccelStepper) with a:

✅ True 32-bit DDS (Direct Digital Synthesis) core
✅ RA-only microstep dithering noise shaping
✅ Shared hardware timer architecture
✅ Sub-arcsecond tracking precision

Designed and optimized for ESP32 / ESP32-S3 using Arduino IDE.

🎯 Key Features
🔁 True DDS Tracking Engine

Phase accumulator based step generation

No interval quantization

Zero cumulative drift

Exact fractional pulse output

🎚 RA Microstep Dithering (Noise Shaping)

First-order sigma-delta modulation

Pushes quantization noise to higher frequencies

Eliminates long-term tracking bias

Reduces star trailing

⚙ Shared Hardware Timer (100kHz base)

One timer for up to 4 motors

Deterministic ISR

No blocking

No FreeRTOS dependency

🚀 Trapezoidal Slewing Engine

Smooth acceleration

Controlled deceleration

No overshoot bounce

Stable GoTo behavior

🔬 Observatory-Level Stability

Tested at sidereal speeds (~11–12 steps/sec)

Clean 20+ second exposures

No mechanical oscillation
