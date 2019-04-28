// -----------------------------------------------------------
uint32_t xscale = 20;																									// How far apart they are
uint32_t yscale = 3;																									// How fast they move
uint8_t indexFire = 0;

void inoise8_fire() {
    for(int i = 0; i < NUM_LEDS; i++) {
        indexFire = inoise8(i*xscale, millis()*yscale*iNrOfLeds / 255);													// X location is constant, but we move along the Y at the rate of millis()
        crgbLeds[i] = ColorFromPalette(crgbCurrentPalette, min(i*(indexFire) >> 6, 255), i * 255 / iNrOfLeds, LINEARBLEND);	// With that value, look up the 8 bit colour palette value and assign it to the current LED.
    }																													// The higher the value of i => the higher up the palette index (see palette definition).
																														// Also, the higher the value of i => the brighter the LED.

    addEffect();
}
// -----------------------------------------------------------
