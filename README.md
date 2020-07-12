# MasterLEDs
A desktop app to control individually addressable leds with an Arduino.

## Requirements
1) The FastLED library for Arduino
2) Your number of LEDs on your strip set in the ardi/led-test.cpp file ("NUM_LEDS = ...") ***AN EVEN NUMBER IS BEST***
3) Your COM port set in the config file (CONFIG.txt) ***THERE MUST BE A BLANK AT THE END OF THE FILE!!!***
4) An arduino with the included sketch uploaded (ardi/led-test.cpp)

## To run
1) Clone this repo
2) Inside the repo dir, run `npm install electron`
3) Run `npm start`
