// -----------------------------------------------------------

void standardTile(){
	uint8_t ui8Brightness = 255;
	uint8_t ui8ColorIndex = gHue;

	for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
		for (int j = i * (beNrOfSidesPerTile[i] * beNrOfLedsCorner); j < (i * (beNrOfSidesPerTile[i] * beNrOfLedsCorner)) + (beNrOfSidesPerTile[i] * beNrOfLedsCorner) - 1; j++) {
			crgbLeds[j] = ColorFromPalette(crgbCurrentPalette, ui8ColorIndex, ui8Brightness, currentBlending);
		}
		ui8ColorIndex += (beNrOfSidesPerTile[i] * beNrOfLedsCorner) * 2;
	}

    addEffect();
}

// -----------------------------------------------------------
