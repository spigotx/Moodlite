// -----------------------------------------------------------

void sinelon(){
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(crgbLeds, iNrOfLeds, 20);
    int pos = beatsin16(13, 0, iNrOfLeds - 1 );
    crgbLeds[pos] += CHSV(gHue, 255, 192);

    addEffect();
}

void sinelonTile(){
    fadeToBlackBy(crgbLeds, iNrOfLeds, 20);
    int pos = beatsin16(13, 0, iNrOfLeds - 1 );
    int ledEnd = 0;
    int ledStart = 0;
    int actualTile = 0;
    for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
        ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        if(pos >= (ledStart - 1) && pos <= (ledEnd - 1)){
            for(int x = 0; x < (ledEnd - ledStart);x++){
                crgbLeds[ledStart + x] += CHSV(gHue, 255, 192);
            }
            i = beNrOfSidesPerTileSize;
        }
    }
    
    addEffect();
}

// -----------------------------------------------------------
