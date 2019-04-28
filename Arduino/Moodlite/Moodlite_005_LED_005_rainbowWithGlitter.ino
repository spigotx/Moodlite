// -----------------------------------------------------------

void rainbowWithGlitter(){
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter){
    if( random8() < chanceOfGlitter) {
        crgbLeds[random16(iNrOfLeds)] += CRGB::White;
    }
}

// -----------------------------------------------------------