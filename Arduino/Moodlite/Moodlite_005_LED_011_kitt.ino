// -----------------------------------------------------------

void kitt(){
    // a colored dot sweeping back and forth, with fading trails
    uint8_t BeatsPerMinute = 62;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    fadeToBlackBy(crgbLeds, iNrOfLeds, 20);
    int pos = beatsin16(13, 0, iNrOfLeds - 1);
    crgbLeds[pos] += ColorFromPalette(crgbCurrentPalette, gHue + (pos * 2), beat - gHue + (pos * 10));

    addEffect();
}

void kittTile(){
    // a colored dot sweeping back and forth, with fading trails
    uint8_t BeatsPerMinute = 62;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    fadeToBlackBy(crgbLeds, iNrOfLeds, 10);
    int pos = beatsin16(13, 0, iNrOfLeds - 1);
    int ledEnd = 0;
    int ledStart = 0;
    int actualTile = 0;
    for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
        ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        if(pos >= (ledStart - 1) && pos <= (ledEnd - 1)){
            for(int x = 0; x < (ledEnd - ledStart);x++){
                crgbLeds[ledStart + x] += ColorFromPalette(crgbCurrentPalette, gHue + (pos * 2), beat - gHue + (pos * 10));
            }
            i = beNrOfSidesPerTileSize;
        }
    }
    
    addEffect();
}

// -----------------------------------------------------------
