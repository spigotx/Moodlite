// -----------------------------------------------------------

void rainbow_beat() {
    uint8_t beatA = beatsin8(17, 0, 255);						// Starting hue
    uint8_t beatB = beatsin8(13, 0, 255);
    fill_rainbow(crgbLeds, iNrOfLeds, (beatA + beatB) / 2, 8);		// Use FastLED's fill_rainbow routine.

    addEffect();
}

// -----------------------------------------------------------
