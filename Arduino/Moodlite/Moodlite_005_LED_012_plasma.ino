// -----------------------------------------------------------

#define qsuba(x, b) ((x>b)?x-b:0)																	// Analog Unsigned subtraction macro. if result <0, then => 0

void plasma(){																						// This is the heart of this program. Sure is short. . . and fast.
    int thisPhase = beatsin8(6, -64, 64);																// Setting phase change for a couple of waves.
    int thatPhase = beatsin8(7, -64, 64);
    
    for (int k = 0; k < iNrOfLeds; k++) {																// For each of the LED's in the strand, set a brightness based on a wave as follows:
    
        int colorIndex = cubicwave8((k * 23) + thisPhase) / 2 + cos8((k * 15) + thatPhase) / 2;					// Create a wave and add a phase change and add another wave with its own phase change.. Hey, you can even change the frequencies if you wish.
        int thisBright = qsuba(colorIndex, beatsin8(7, 0, 96));										// qsub gives it a bit of 'black' dead space by setting sets a minimum value. If colorIndex < current value of beatsin8(), then bright = 0. Otherwise, bright = colorIndex..
        
        crgbLeds[k] = ColorFromPalette(crgbCurrentPalette, colorIndex, thisBright, LINEARBLEND);	// Let's now add the foreground colour.
    }

    addEffect();
}

void plasmaTile(){
    int thisPhase = beatsin8(6, -64, 64);
    int thatPhase = beatsin8(7, -64, 64);
    
    int ledEnd = 0;
    int ledStart = 0;
    for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
        int colorIndex = cubicwave8((i * 23) + thisPhase) / 2 + cos8((i * 15) + thatPhase) / 2;
        int thisBright = qsuba(colorIndex, beatsin8(7, 0, 96));
        ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        for(int x = 0; x < (ledEnd - ledStart);x++){
            crgbLeds[ledStart + x] = ColorFromPalette(crgbCurrentPalette, colorIndex, thisBright, LINEARBLEND);
        }
    }
    
    addEffect();
}
// -----------------------------------------------------------
