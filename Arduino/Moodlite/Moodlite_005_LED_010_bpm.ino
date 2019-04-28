// -----------------------------------------------------------

void bpm(){
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for(int i = 0; i < iNrOfLeds; i++) { //9948
        crgbLeds[i] = ColorFromPalette(crgbCurrentPalette, gHue + (i * 2), beat - gHue + (i * 10));
    }

    addEffect();
}

void bpmTile(){
    fadeToBlackBy(crgbLeds, iNrOfLeds, 10);

    uint8_t BeatsPerMinute = 31;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    
    int ledEnd = 0;
    int ledStart = 0;
    for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
        ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        for(int x = 0; x < (ledEnd - ledStart);x++){
            crgbLeds[ledStart + x] = ColorFromPalette(crgbCurrentPalette, gHue + (i * 2), beat - gHue + (i * 10));
        }
    }
    
    addEffect();
}

// -----------------------------------------------------------	
