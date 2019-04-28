// -----------------------------------------------------------
uint8_t mul1 = 7;                                            // Frequency, thus the distance between waves
int wave1=0;                                                  // Current phase is calculated.
int wave2=0;
int wave3=0;

void three_sin() {
  
  wave1 += beatsin8(10,-4,4);
  wave2 += beatsin8(15,-2,2);
  wave3 += beatsin8(12,-3, 3);

  for (int k=0; k<iNrOfLeds; k++) {
    uint8_t tmp = sin8(mul1*k + wave1) + sin8(mul1*k + wave2) + sin8(mul1*k + wave3);
    crgbLeds[k] = ColorFromPalette(crgbCurrentPalette, tmp, 255);
  }

    addEffect();
  
}

void three_sinTile(){
  
    wave1 += beatsin8(10,-4,4);
    wave2 += beatsin8(15,-2,2);
    wave3 += beatsin8(12,-3, 3);

    int ledEnd = 0;
    int ledStart = 0;
    for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
        uint8_t tmp = sin8(mul1*i + wave1) + sin8(mul1*i + wave2) + sin8(mul1*i + wave3);
        
        ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        for(int x = 0; x < (ledEnd - ledStart);x++){
            crgbLeds[ledStart + x] = ColorFromPalette(crgbCurrentPalette, tmp, 255);
        } 
    }
    
    addEffect();
}
// -----------------------------------------------------------
