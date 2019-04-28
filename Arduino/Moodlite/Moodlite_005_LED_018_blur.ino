// -----------------------------------------------------------
void blur() {

  uint8_t blurAmount = dim8_raw( beatsin8(3,64, 192) );       // A sinewave at 3 Hz with values ranging from 64 to 192.
  blur1d( crgbLeds, iNrOfLeds, blurAmount);                        // Apply some blurring to whatever's already on the strip, which will eventually go black.
  
  uint8_t  i = beatsin8(  9, 0, iNrOfLeds);
  uint8_t  j = beatsin8( 7, 0, iNrOfLeds);
  uint8_t  k = beatsin8(  5, 0, iNrOfLeds);
  
  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis();  
  crgbLeds[(i+j)/2] = CHSV( ms / 29, 200, 255);
  crgbLeds[(j+k)/2] = CHSV( ms / 41, 200, 255);
  crgbLeds[(k+i)/2] = CHSV( ms / 73, 200, 255);
  crgbLeds[(k+i+j)/3] = CHSV( ms / 53, 200, 255);

    addEffect();

}
// -----------------------------------------------------------
