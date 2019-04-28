// -----------------------------------------------------------

void confetti(){
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy(crgbLeds, iNrOfLeds, 10);
    int pos = random16(iNrOfLeds);
    crgbLeds[pos] += CHSV(gHue + random8(64), 200, 255);

    addEffect();
}

void confettiTile(){
    fadeToBlackBy(crgbLeds, iNrOfLeds, 10);
    int pos = random16(iNrOfLeds);
    int ledEnd = 0;
    int ledStart = 0;
    int actualTile = 0;
    for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
        ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        if(pos >= (ledStart - 1) && pos <= (ledEnd - 1)){
            int ran = random8(64);
            for(int x = 0; x < (ledEnd - ledStart);x++){
                crgbLeds[ledStart + x] += CHSV(gHue + ran, 200, 255);
            }
            i = beNrOfSidesPerTileSize;
        }
    }
    
    addEffect();
}

// -----------------------------------------------------------
