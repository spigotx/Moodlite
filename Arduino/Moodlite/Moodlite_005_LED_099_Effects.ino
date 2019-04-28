void addEffect(){
    switch (iLedEffect) {
        case 1000: //Glitter
            glitter(80);
            break;

        case 1001: //Dot beat
            dotBeat();
            break;

        case 1002: //Lightnings
            lightnings();
            break;

        case 1003: //Meteor shower
            meteorRain(0xff,0xff,0xff,10, 64, true, 30);
            break;
    }
}

// -----------------------------------------------------------
void glitter(fract8 chanceOfGlitter){
    if( random8() < chanceOfGlitter) {
        crgbLeds[random16(iNrOfLeds)] += CRGB::White;
    }
}
// -----------------------------------------------------------

// -----------------------------------------------------------
uint8_t fadeval = 224; 
uint8_t bpmDotBeat = 30;
void dotBeat() {
  uint8_t inner = beatsin8(bpmDotBeat, iNrOfLeds/4, iNrOfLeds/4*3);    // Move 1/4 to 3/4
  uint8_t outer = beatsin8(bpmDotBeat, 0, iNrOfLeds-1);               // Move entire length
  uint8_t middle = beatsin8(bpmDotBeat, iNrOfLeds/3, iNrOfLeds/3*2);   // Move 1/3 to 2/3

  crgbLeds[middle] = CRGB::Purple;
  crgbLeds[inner] = CRGB::Blue;
  crgbLeds[outer] = CRGB::Aqua;

  nscale8(crgbLeds,iNrOfLeds,fadeval);                             // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);

}
// -----------------------------------------------------------

// -----------------------------------------------------------
uint8_t frequency = 50;                                       // controls the interval between strikes
uint8_t flashes = 8;                                          //the upper limit of flashes per strike
unsigned int dimmer = 1;
uint8_t ledstart;
uint8_t ledlen;
void lightnings() {
  ledstart = random8(iNrOfLeds);                               // Determine starting location of flash
  ledlen = random8(iNrOfLeds-ledstart);                        // Determine length of flash (not to go beyond NUM_LEDS-1)
  
  for (int flashCounter = 0; flashCounter < random8(3,flashes); flashCounter++) {
    if(flashCounter == 0) dimmer = 5;                         // the brightness of the leader is scaled down by a factor of 5
    else dimmer = random8(1,3);                               // return strokes are brighter than the leader
    
    fill_solid(crgbLeds+ledstart,ledlen,CHSV(255, 0, 255/dimmer));
    FastLED.show();                       // Show a section of LED's
    delay(random8(4,10));                                     // each flash only lasts 4-10 milliseconds
    fill_solid(crgbLeds+ledstart,ledlen,CHSV(255,0,0));           // Clear the section of LED's
    FastLED.show();
    
    if (flashCounter == 0) delay (150);                       // longer delay until next flash after the leader
    
    delay(50+random8(100));                                   // shorter delay between strokes  
  } // for()
  
  delay(random8(frequency)*100);                              // delay between strikes
}
// -----------------------------------------------------------

// -----------------------------------------------------------
void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  setAll(0,0,0);
  
  for(int i = 0; i < iNrOfLeds+iNrOfLeds; i++) {
    // fade brightness all LEDs one step
    for(int j=0; j<iNrOfLeds; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
    
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <iNrOfLeds) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      } 
    }
   
    FastLED.show();
    delay(SpeedDelay);
  }
}
void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < iNrOfLeds; i++ ) {
    setPixel(i, red, green, blue); 
  }
  FastLED.show();
}
void fadeToBlack(int ledNo, byte fadeValue) {
   crgbLeds[ledNo].fadeToBlackBy( fadeValue );
}
void setPixel(int Pixel, byte red, byte green, byte blue) {
   crgbLeds[Pixel].r = red;
   crgbLeds[Pixel].g = green;
   crgbLeds[Pixel].b = blue;
}
// -----------------------------------------------------------
