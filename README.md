# MasterLEDs
A desktop app to control individually addressable leds with an Arduino.

## Requirements
1) The FastLED library for Arduino
2) Your number of LEDs on your strip set in the ardi/led-test.cpp file ("NUM_LEDS = ...") ***AN EVEN NUMBER IS BEST***
3) Your COM port set in the config file (CONFIG.txt) ***THERE MUST BE A BLANK AT THE END OF THE FILE!!!***
4) An arduino with the included sketch uploaded (ardi/led-test.cpp)
5) Your LED strip wired up so that the data pin is connected to PWM pin 7 on the arduino

## To run source
1) Install npm
2) Clone this repo
3) Inside the repo dir, run `npm install electron`
4) Run `npm start`
