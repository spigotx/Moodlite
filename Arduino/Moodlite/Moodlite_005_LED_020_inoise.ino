// -----------------------------------------------------------
void inoise_8() {
  
  for(int i = 0; i < iNrOfLeds; i++) {                                      // Just ONE loop to fill up the LED array as all of the pixels change.
    uint8_t index = inoise8(i*xscale, iRndINoise+i*yscale) % 255;                // Get a value from the noise function. I'm using both x and y axis.
    crgbLeds[i] = ColorFromPalette(crgbCurrentPalette, index, 255, currentBlending);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }
  
  iRndINoise += beatsin8(10,1,4);                                                // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.
                                                                           // In some sketches, I've used millis() instead of an incremented counter. Works a treat.

    addEffect();
}

void inoise_8Tile(){
    int ledEnd = 0;
    int ledStart = 0;
    for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
        uint8_t index = inoise8(i*xscale, iRndINoise+i*yscale) % 255;
        ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        for(int x = 0; x < (ledEnd - ledStart);x++){
            crgbLeds[ledStart + x] = ColorFromPalette(crgbCurrentPalette, index, 255, currentBlending);
        }
    }
    iRndINoise += beatsin8(10,1,4); 
    
    addEffect();
}

// -----------------------------------------------------------
