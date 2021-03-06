/*
Code for controlling individual addressable led lights with an Arduino.

PRO TIP: When a value is set to a light, in order to actually see the changes,
         one must call FastLED.show().

void clearAll():
    Sets all leds to display nothing.
void setAllSolidColor():
    Runs through each led and sets its value to color_to_set.
void updateColorTimer(int rate_ms):
    If the user specified rate in milliseconds is greater than the delta
    system time, then it will toggle the color from CHSV_OFF to a specified
    color and set it to color_to_set.
int getMorseMS(char ch):
    Returns the proper time given a dot, dash, or a space.
void illuminate():
    Writes values to the leds in a certain way based on led_mode.
    If led_mode is SOLID, it calls setAllSolidColor(), without changing any
    values of color_to_set.
    If led_mode is BLINK, it calls updateColorTimer() to correctly set the
    color that is then set in setAllSolidColor().
    If led_mode is TRAIL, it calls updateColorTimer() to correctly set the
    color that is then used when it sets the first LED.  After that, starting
    from the last LED, the value of the current LED is set to the value of the
    LED before it.
    if led_mode is TRAIL_FROM_MIDDLE, it does a similar procedure as TRAIL,
    except it has two trails, both originating from the middle of the strand of
    lights that go to each end of the strip.
    If led_mode is WAVE, all of the leds are set so that the entire 360 degrees
    are spread out evenly between them.  Then, wave_counter is set to itself
    plus (255.0 / 300.0) * wave_speed_scalar.  This insures that we get all 360
    degrees of hue.  wave_speed_scalar is used to either speed up or slow down
    the effect.
    If led_mode if MORSE_CODE, then the leds flash the morse code specified by
    morse.  It does this by checking if we are in between a letter, character,
    or are in the middle of displaying a character.  If we are in between a
    letter, and our delta system time is greater than space_between_letter_ms,
    we will move on to the next character.  If we are in between a character,
    then we do the same thing as if we are in between a letter, except with the
    proper time, space_ms.  If we are in the middle of displaying a character,
    we will set color_to_set to a specified color, else, we will set it to
    nothing.  We then call setAllSolidColor().  Finally, we use FastLED.show()
    to display our lights.
void setup():
    Initializes our LEDs, clears them, and displays the cleared LEDs.  We then
    specify the mode we want to use.  Finally, we delay the leds so that they
    have a chance to react after being initialized, then reset our system
    timer.
void loop():
    This funtion is called over and over by the Arduino until the reset button
    is pressed; it only calls illuminate().
*/

#include "Arduino.h"
#include <FastLED.h>
#include "Utils.h"

typedef unsigned int uint;

const int LED_PIN = 7;
const int NUM_LEDS = 300;

double rate_ms = 1000;

const int WPM = 12;
char* morse = (char*)malloc(sizeof(char));
int index = 0;
bool inBetweenChar = false;
bool inBetweenLetter = false;
int dot_ms = 1200 / WPM;
int dash_ms = dot_ms * 3;
int space_ms = dot_ms;
int space_between_letter_ms = dot_ms * 7;
const bool USING_RAMP = true;

long timer;
bool was_on = false;
double wave_counter = 0.0;
double wave_speed_scalar = 1.0;

double intensity_scalar = 1.0;

bool out = true;

enum modes {
    BLINK,
    TRAIL,
    SOLID,
    WAVE,
    MORSE_CODE,
    TRAIL_FROM_MIDDLE,
    MUSIC_SIMPLE,
    MUSIC,
    TESTING,
    OFF
} led_mode, last_mode;

CRGB leds[NUM_LEDS];

const CHSV CHSV_OFF = CHSV(0, 0, 0);
const CHSV CHSV_RED_FULL = CHSV(0, 255, 255);

CHSV solid_color = CHSV_OFF;
CHSV color_to_set = CHSV_RED_FULL;

CRGB solid_color_rgb = CRGB(255, 0, 0);

void clearAll() {

    FastLED.clear();
}

void ClearSerial() {

    while (Serial.available() > 0) { char ch = Serial.read(); }
}

void FlashByte(byte b) {

    for (int j = 0; j < 8; j++) {

        digitalWrite(LED_BUILTIN, HIGH);
        if ((b >> (7 - j)) & 0x01) {

            delay(500);
        }
        else {

            delay(100);
        }
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
    }

    delay(2000);
}

void FlashNumBytes() {

    FlashByte(Serial.available());
}

void setAllSolidColor() {

    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(color_to_set.hue, color_to_set.sat, color_to_set.val * intensity_scalar);
    }
}

void setAllSolidColorRGB() {

    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = solid_color_rgb;
    }
}

void updateColorTimer(int rate_ms) {

    if ((millis() - timer) >= rate_ms) {
            
        if (!was_on){
            
            color_to_set = CHSV_RED_FULL;
            was_on = true;
        }
        else {

            color_to_set = CHSV_OFF;
            was_on = false;
        }

        timer = millis();
    }
}

void updateWaveCounter() {

    wave_counter += (255.0 / (double)NUM_LEDS) * wave_speed_scalar;
}

int getMorseMS(char ch) {

    if (ch == '.') { return dot_ms; }
    else if (ch == '-') { return dash_ms; }
    else { return 5000; }
}

void illuminate() {

    if (led_mode == OFF) {

        clearAll();
    }

    else if (led_mode == SOLID) {

        setAllSolidColorRGB();
    }

    else if (led_mode == BLINK) {

        updateColorTimer(rate_ms);
        setAllSolidColor();
    }

    else if (led_mode == TRAIL) {

        updateColorTimer(rate_ms);

        leds[0] = CHSV(color_to_set.hue, color_to_set.sat, 255.0 * intensity_scalar);
        for (int i = NUM_LEDS - 1; i > 0; i--) {

            leds[i] = leds[i - 1];
        }
    }

    else if (led_mode == TRAIL_FROM_MIDDLE) {

        updateColorTimer(rate_ms);

        if (out) {
            
            if (color_to_set != CHSV_OFF) {
                
                leds[NUM_LEDS / 2] = solid_color_rgb;
                leds[NUM_LEDS / 2 - 1] = solid_color_rgb;
            }
            else {

                leds[NUM_LEDS / 2] = CHSV_OFF;
                leds[NUM_LEDS / 2 - 1] = CHSV_OFF;
            }
            for (int i = 0; i < NUM_LEDS / 2 - 1; i++) {

                leds[i] = leds[i + 1];
            }
            for (int i = NUM_LEDS - 1; i > NUM_LEDS / 2; i--) {

                leds[i] = leds[i - 1];
            }
        }
        else {

            if (color_to_set != CHSV_OFF) {

                leds[NUM_LEDS - 1] = solid_color_rgb;
                leds[0] = solid_color_rgb;
            }
            else {

                leds[NUM_LEDS - 1] = CHSV_OFF;
                leds[0] = CHSV_OFF;
            }
            for (uint i = NUM_LEDS / 2 - 1; i > 0; i--) {

                leds[i] = leds[i - 1];
            }
            for (uint i = NUM_LEDS / 2; i < NUM_LEDS - 1; i++) {

                leds[i] = leds[i + 1];
            }
        }
    }

    else if (led_mode == WAVE) {

        for (int i = 0; i < NUM_LEDS; i++) {

            leds[i] = CHSV((i * 255.0 / 300.0) + wave_counter * wave_speed_scalar, 255, 255);
        }
        updateWaveCounter();
    }

    else if (led_mode == MORSE_CODE) {

        if (inBetweenLetter) {

            if ((millis() - timer) >= space_between_letter_ms) {

                //Example: --.--.. --.-
                //               | |
                //We are here----| |--|
                //We need to go here--|
                index++;
                index++;
                inBetweenLetter = false;
                timer = millis();
            }
        }

        else if (inBetweenChar) {

            if ((millis() - timer) >= space_ms) {

                index++;
                inBetweenChar = false;
                timer = millis();
            }
        }

        else if ((millis() - timer) >= getMorseMS(morse[index])) {

            if (morse[index + 1] == ' ') {

                inBetweenLetter = true;
            }
            else if (index + 1 > strlen(morse) - 1) {

                index = -2;
                inBetweenLetter = true;
            }
            else {

                inBetweenChar = true;
            }
            
            timer = millis();
        }

        if (inBetweenChar || inBetweenLetter) {

            color_to_set = CHSV_OFF;
        }
        
        else {

            color_to_set = CHSV_RED_FULL;
        }

        setAllSolidColor();
    }

    else if (led_mode == MUSIC_SIMPLE) {

        unsigned char buffer[128] = {0};

        //Make sure there is data to be read :)
        if (Serial.available() >= 0) {

            //Read until the number of bytes has been read, or until the
            //timeout has been reached
            Serial.readBytes(buffer, 128);
            ClearSerial();

            for (int i = 0; i < 128; i++) {

                leds[i] = CHSV(i * 2 + wave_counter, 255, (int)buffer[127 - i]);
            }
            for (int i = 128; i < 256; i++) {

                leds[i] = CHSV((255 - i) * 2 + wave_counter, 255, (int)buffer[i - 128]);
            }
        }
    }

    else if (led_mode == MUSIC) {

        unsigned char buffer[128] = {0};
        double ledsEffects[128] = {0};

        //Make sure there is data to be read :)
        if (Serial.available() >= 0) {

            //Read until the number of bytes has been read, or until the
            //timeout has been reached
            Serial.readBytes(buffer, 128);
            ClearSerial();

            for (int i = 0; i < 128; i++) {

                int masterVal = (int)buffer[i];
                double masterScalar = masterVal / 255.0;

                if (masterVal != 0) {

                    //middle
                    ledsEffects[i] = masterScalar / 1.0;
                    
                    if (USING_RAMP) {
                        
                        int numOfFades = RoundLit(masterVal * 4 / 255.0);
                        int valToSet;
                        int valAlready;
                        //left side
                        for (int j = 0; j < numOfFades; j++) {
                            
                            valToSet = j + 2;
                            valAlready = ledsEffects[i - j - 1];
                            //                                          less than bc it is the bottom of a fraction
                            if ((i - j - 1 >= 0 && i - j - 1 <= 127) && ((valToSet < valAlready) || valAlready == 0)) {

                                ledsEffects[i - j - 1] = masterScalar / valToSet;
                            }
                        }
                        //right side
                        for (int j = 0; j < numOfFades; j++) {
                            
                            valToSet = j + 2;
                            valAlready = ledsEffects[i + j + 1];
                            //                                          less than bc it is the bottom of a fraction
                            if ((i + j + 1 >= 0 && i + j + 1 <= 127) && ((valToSet < valAlready) || valAlready == 0)) {

                                ledsEffects[i + j + 1] = masterScalar / valToSet;
                            }
                        }
                    }
                }
                else if (ledsEffects[i] == 0 ) ledsEffects[i] = 0;
            }

            for (int i = 0; i < 128; i++) {

                leds[i] = CHSV(i * 2 + wave_counter, 255, (ledsEffects[127 - i] == 0) ? 0 : (int)(ledsEffects[127 - i] * 256) - 1);
            }
            for (int i = 128; i < 256; i++) {

                leds[i] = CHSV((255 - i) * 2 + wave_counter, 255, (ledsEffects[i - 128] == 0) ? 0 : (int)(ledsEffects[i - 128] * 256) - 1);
            }
        }
        updateWaveCounter();
    }

    else if (led_mode == TESTING) {

        for (int i = 0; i < 255; i++) { leds[i] = CHSV(0, 255, i); }
    }

    FastLED.show();
}

void setup() {

    Serial.begin(57600);
    Serial.setTimeout(200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    clearAll();
    FastLED.show();

    led_mode = OFF;
    last_mode = OFF;

    delay(500);

    timer = millis();
}

unsigned char c;
void loop() {

    //Apparently, FastLED.show() disables interrupts, which somehow deletes
    //a large portion of the data in the serial input buffer (OOOOOOOF)
    Serial.write(127);
    int incoming = Serial.read();
    if (incoming != 0 && incoming != -1) {

        byte b[34] = { 0 };
        Serial.readBytes(b, 34);

        c = b[0];
    
        switch (c) {

        case 's': {
            solid_color_rgb = CRGB(b[1], b[2], b[3]);
            led_mode = SOLID;
            break;
        }

        case 'w': {
            double* buffer = (double*)malloc(sizeof(double));
            memcpy(buffer, b + 1, sizeof(double));
            wave_speed_scalar = 1.0 / (*buffer / 8.0);
            wave_counter = 0.0;
            led_mode = WAVE;
            break;
        }

        case 't': {
            double* buffer = (double*)malloc(sizeof(double));
            memcpy(buffer, b + 1, sizeof(double));
            rate_ms = *buffer;
            solid_color_rgb = CRGB(*(b + 5), *(b + 6), *(b + 7));
            out = *(b + 8);
            led_mode = TRAIL_FROM_MIDDLE;
            break;
        }

        case 'm': {
            unsigned char text_len = b[1];
            free(morse);
            morse = (char*)malloc((text_len + 1) * sizeof(char));
            memcpy(morse, b + 2, text_len * sizeof(char));
            morse[text_len] = 0; //last 0 is the null-terminator
            double* buffer = (double*)malloc(sizeof(double));
            memcpy(buffer, b + 2 + text_len, sizeof(double));
            dot_ms = (int)*buffer != 0 ? (int)*buffer : 1200 / WPM;
            memcpy(buffer, b + 2 + text_len + 4, sizeof(double));
            dash_ms = (int)*buffer != 0 ? (int)*buffer : dot_ms * 3;
            memcpy(buffer, b + 2 + text_len + 8, sizeof(double));
            space_ms = (int)*buffer != 0 ? (int)*buffer : dot_ms;
            memcpy(buffer, b + 2 + text_len + 16, sizeof(double));
            space_between_letter_ms = (int)*buffer != 0 ? (int)*buffer : dot_ms * 7;
            led_mode = MORSE_CODE;
            break;
        }

        case 'i': {
            intensity_scalar = b[1] / 100.0;
            FastLED.setBrightness(intensity_scalar * 255.0);
            break;
        }

        case 0: {
            if (led_mode != OFF) last_mode = led_mode;
            led_mode = OFF;
            break;
        }

        case 1: {
            led_mode = last_mode;
            break;
        }
            
        default:
            led_mode = BLINK;
            break;
        }
        ClearSerial();
    }

    illuminate();
}

/**
    for (int i = 0; i < 34; i++) {

        char s[3] = { 0 };
        sprintf(s, "%d ", b[i]);
        Serial.write(s);
    }
*/