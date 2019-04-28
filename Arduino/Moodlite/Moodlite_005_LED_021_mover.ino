// -----------------------------------------------------------
void mover() {                                             // running red stripe.
    for (int i = 0; i < iNrOfLeds; i++) {
        uint8_t red = (millis() / 5) + (i * 12);                    // speed, length
        if (red > 128) red = 0;
            crgbLeds[i] = ColorFromPalette(crgbCurrentPalette, red, red, currentBlending);
    }

    addEffect();
}

void moverTile(){
    int ledEnd = 0;
    int ledStart = 0;
    for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
        uint8_t red = (millis() / 5) + (i * 12);                    // speed, length
        ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        if (red > 128) red = 0;
        for(int x = 0; x < (ledEnd - ledStart);x++){
            crgbLeds[ledStart + x] = ColorFromPalette(crgbCurrentPalette, red, red, currentBlending);
        }
    }
    
    addEffect();
}
// -----------------------------------------------------------
