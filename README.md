# Axis Project
## Overview 
A guitar pedal that you can change while you play! The Axis guitar will modulate
an effect of your choice based on guitar movement. Two different effects can be
modulated at the same time by choosing to modulate on different rotational
axes.
### Features
- Modulate effects based on movement
- Two effects can be modulated
- Effects are programmable
## Schematics and PCB Designs
Schematics and PCB designs are created using KiCad. The project files for both
the guitar unit and the floor unit can be found within the PCB folder. 
## Audio ESP32 
The ESP32 microcontroller on the ground. This is responsible for connecting to
the guitar unit and relaying sensor data to the daisy seed via I2C.
## IMU ESP32 
The ESP32 microcontroller on the guitar. This is responsible for advertising
itself over bluetooth and relaying sensor data to the devices that it is
connected to.
## Daisy Seed
The Daisy seed is responsible for handling the audio and modulating it based on
the sensor data. After an effect is programmed the daisy seed will modulate the
audio input.
