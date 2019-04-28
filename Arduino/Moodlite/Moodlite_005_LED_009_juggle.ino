// -----------------------------------------------------------

void juggle(){
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(crgbLeds, iNrOfLeds, 20);
    byte dothue = 0;
    for(int i = 0; i < 8; i++) {
        crgbLeds[beatsin16(i + 7, 0, iNrOfLeds - 1 )] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }

    addEffect();
}

void juggleTile(){
    fadeToBlackBy(crgbLeds, iNrOfLeds, 10);
    byte dothue = 0;
    for(int i = 0; i < 8; i++) {
        int pos = beatsin16(i + 7, 0, iNrOfLeds - 1 );
        int ledEnd = 0;
        int ledStart = 0;
        int actualTile = 0;
        for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
            ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
            ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
            if(pos >= (ledStart - 1) && pos <= (ledEnd - 1)){
                int ran = random8(64);
                for(int x = 0; x < (ledEnd - ledStart);x++){
                    crgbLeds[ledStart + x] |= CHSV(dothue, 200, 255);
                }
                dothue += 32;
                i = beNrOfSidesPerTileSize;
            }
        }
    }
    
    addEffect();
}

void juggleTileV2(){
    fadeToBlackBy(crgbLeds, iNrOfLeds, 10);
    byte dothue = 0;
    for(int i = 0; i < 8; i++) {
        //DEBUGF("beNrOfSidesPerTileSize:%u%\n\r",beNrOfSidesPerTileSize);
        int pos = beatsin16(i + 7, 0, beNrOfSidesPerTileSize - 1 );
        //DEBUGF("pos:%u%\n\r",pos);
        int ledEnd = 0;
        int ledStart = 0;
        for (int i = 0; i <= pos; i++) {
            ledEnd += (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        }
        ledStart = ledEnd - (beNrOfSidesPerTile[pos] * beNrOfLedsCorner);
        ledEnd -= 1;
        //DEBUGF("ledStart:%u%\n\r",ledStart);
        //DEBUGF("ledEnd:%u%\n\r",ledEnd);
        for(int x = 0; x < (ledEnd - ledStart);x++){
            crgbLeds[ledStart + x] |= CHSV(dothue, 200, 255);
        }
        dothue += 32;
    }
    
    addEffect();
}

// -----------------------------------------------------------
