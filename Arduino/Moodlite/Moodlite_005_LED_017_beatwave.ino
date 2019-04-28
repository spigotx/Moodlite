// -----------------------------------------------------------

void beatwave() {
  
    uint8_t wave1 = beatsin8(9, 0, 255);                        // That's the same as beatsin8(9);
    uint8_t wave2 = beatsin8(8, 0, 255);
    uint8_t wave3 = beatsin8(7, 0, 255);
    uint8_t wave4 = beatsin8(6, 0, 255);
    
    for (int i=0; i<iNrOfLeds; i++) {
        crgbLeds[i] = ColorFromPalette( crgbCurrentPalette, i+wave1+wave2+wave3+wave4, 255, currentBlending); 
    }

    addEffect();
  
}

void beatwaveTile(){
    uint8_t wave1 = beatsin8(9, 0, 255);                        // That's the same as beatsin8(9);
    uint8_t wave2 = beatsin8(8, 0, 255);
    uint8_t wave3 = beatsin8(7, 0, 255);
    uint8_t wave4 = beatsin8(6, 0, 255);
    
    int ledEnd = 0;
    int ledStart = 0;
    for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
        ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        for(int x = 0; x < (ledEnd - ledStart);x++){
            crgbLeds[ledStart + x] = ColorFromPalette( crgbCurrentPalette, i+wave1+wave2+wave3+wave4, 255, currentBlending); 
        }
    }
    
    addEffect();
}
// -----------------------------------------------------------
