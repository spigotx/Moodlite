// --------------------- LED functions -----------------------

// Turn off LEDs
void turnOffLeds(byte beStartNr, byte beEndNr) {
	for (int i = beStartNr; i <= beEndNr; i++) {
		crgbLeds[i] = CRGB::Black;
	}
}

// Set LEDs color
void setLedsColor(byte beStartNr, byte beEndNr, CRGB crgbColor) {
	for (int i = beStartNr; i <= beEndNr; i++) {
		crgbLeds[i] = crgbColor;
	}
}

// Change LED color pattern parameters
void changeLedColorPatternParameters(byte beColorParameter) {
    CRGB crgbPurple = CHSV(HUE_PURPLE, 255, 255);
    CRGB crgbGreen = CHSV(HUE_GREEN, 255, 255);
    CRGB crgbBlack = CRGB::Black;
    
    switch (beLedColorPattern) {
		case 1: //Cloud
			crgbCurrentPalette = CloudColors_p;
			break;

		case 2: //Lava
			crgbCurrentPalette = LavaColors_p;
			break;

		case 3: //Ocean
			crgbCurrentPalette = OceanColors_p;
			break;

		case 4: //Forest
			crgbCurrentPalette = ForestColors_p;
			break;

		case 5: //Rainbow
			crgbCurrentPalette = RainbowColors_p;
			break;

		case 6: //Rainbow Stripes
			crgbCurrentPalette = RainbowStripeColors_p;
			break;

		case 7: //Party
			crgbCurrentPalette = PartyColors_p;
			break;

		case 8: //Heat
			crgbCurrentPalette = HeatColors_p;
			break;

		case 9: //Fire
			crgbCurrentPalette = CRGBPalette16(
			   CRGB::Black, CRGB::Black, CRGB::Black, CHSV(0, 255,4),
			   CHSV(0, 255, 8), CRGB::Red, CRGB::Red, CRGB::Red,                                   
			   CRGB::DarkOrange,CRGB::Orange, CRGB::Orange, CRGB::Orange,
			   CRGB::Yellow, CRGB::Yellow, CRGB::Gray, CRGB::Gray);
			break;

		case 10: //Random
			for (int i = 0; i < 16; i++) {
				crgbCurrentPalette[i] = CHSV(random8(), 255, random8());
			}
			break;

		case 11: //Purple and Green    
			crgbCurrentPalette = CRGBPalette16(
				crgbGreen, crgbGreen, crgbBlack, crgbBlack,
				crgbPurple, crgbPurple, crgbBlack, crgbBlack,
				crgbGreen, crgbGreen, crgbBlack, crgbBlack,
				crgbPurple, crgbPurple, crgbBlack, crgbBlack
			);
			break;

		case 12: //Red, White and Blue
			crgbCurrentPalette = myRedWhiteBluePalette_p;
			break;
    }
    
	// Change also pattern
	if (beLedPattern == 5) {
		setFirePattern();
	}
}

// Switch selected pattern
void switchLedPattern(byte bePatternParameter) {
	if (bePatternParameter >= 0 && bePatternParameter <= 99)
		changeLedPatternParameters(bePatternParameter);
	else if (bePatternParameter >= 99 && bePatternParameter < 199)
		changeLedFixedPatternParameters(bePatternParameter);
	else
		;
}

// Change LED pattern parameters
void changeLedPatternParameters(byte bePatternParameter) {
    switch (beLedPattern) {
		case 1: //Fixed
			setPattern(0);
			fill_solid(crgbLeds, iNrOfLeds, crgbLedColors);
			FastLED.show();
			break;

		case 2: //Standard
			currentBlending = LINEARBLEND;
			setPattern(1);
			break;

		case 3: //BPM
			currentBlending = LINEARBLEND;
			setPattern(5);
			break;

		case 4: //KITT
			currentBlending = LINEARBLEND;
			setPattern(6);
			break;

		case 5: //Plasma
			currentBlending = LINEARBLEND;
			setPattern(7);
			break;

		case 6: //Fire
			currentBlending = LINEARBLEND;
			setFirePattern();
			setPattern(9);
			break;

        case 7: //Standard Tile
            currentBlending = LINEARBLEND;
            setPattern(13);
            break;
            
        case 8: //Beatwave
            currentBlending = LINEARBLEND;
            setPattern(14);
            break;
            
        case 9: //Fillnoise
            iRndINoise = random16(12345);
            currentBlending = LINEARBLEND;
            setPattern(17);
            break;
            
        case 10: //Mover
            currentBlending = LINEARBLEND;
            setPattern(18);
            break;
            
        case 11: //Noise1
            currentBlending = LINEARBLEND;
            setPattern(19);
            break;
            
        case 12: //Noise2
            currentBlending = LINEARBLEND;
            setPattern(20);
            break;
            
        case 13: //Noise3
            currentBlending = LINEARBLEND;
            setPattern(21);
            break;
            
        case 14: //Three sin
            currentBlending = LINEARBLEND;
            setPattern(22);
            break;

       case 15: //BMP Tiles
            currentBlending = LINEARBLEND;
            setPattern(26);
            break;
            
        case 16: //Kitt Tiles
            currentBlending = LINEARBLEND;
            setPattern(27);
            break;
            
        case 17: //Plasma Tiles
            currentBlending = LINEARBLEND;
            setPattern(28);
            break;
            
        case 18: //Beatwave Tiles
            currentBlending = LINEARBLEND;
            setPattern(29);
            break;
            
        case 19: //Fillnoise Tiles
            currentBlending = LINEARBLEND;
            setPattern(30);
            break;
            
        case 20: //Mover Tiles
            currentBlending = LINEARBLEND;
            setPattern(31);
            break;
            
        case 21: //Noise1 Tiles
            currentBlending = LINEARBLEND;
            setPattern(32);
            break;
            
        case 22: //Noise2 Tiles
            currentBlending = LINEARBLEND;
            setPattern(33);
            break;
            
        case 23: //Noise3 Tiles
            currentBlending = LINEARBLEND;
            setPattern(34);
            break;
            
        case 24: //Three sin Tiles
            currentBlending = LINEARBLEND;
            setPattern(35);
            break;							

		case 99:
			FastLED.clear();
			FastLED.show();

			// Restart ESP to update values
			ESP.restart();
			break;
    }
}

// Change LED fixed pattern parameters
void changeLedFixedPatternParameters(byte bePatternParameter) {
	switch (beLedPattern) {
		case 100: //Confetti
			currentBlending = LINEARBLEND;
			setPattern(2);
			break;

		case 101: //Sinelon
			currentBlending = LINEARBLEND;
			setPattern(3);
			break;

		case 102: //Juggle
			currentBlending = LINEARBLEND;
			setPattern(4);
			break;

		case 103: //Blendwave
			currentBlending = LINEARBLEND;
			setPattern(8);
			break;

		case 104: //Rainbow beat
			currentBlending = LINEARBLEND;
			setPattern(10);
			break;

		case 105: //Full rainbow
			currentBlending = LINEARBLEND;
			setPattern(11);
			break;

		case 106: //Rainbow with glitter
			currentBlending = LINEARBLEND;
			setPattern(12);
			break;

        case 107: //Blendwave
            currentBlending = LINEARBLEND;
            setPattern(15);
            break;

        case 108: //Fill grad
            currentBlending = LINEARBLEND;
            setPattern(16);
            break;

        case 109: //Confetti tiles
            currentBlending = LINEARBLEND;
            setPattern(23);
            break;

        case 110: //Sinelon tiles
            currentBlending = LINEARBLEND;
            setPattern(24);
            break;

        case 111: //Juggle tiles
            currentBlending = LINEARBLEND;
            setPattern(25);
            break;	

		case 99:
			FastLED.clear();
			FastLED.show();

			// Restart ESP to update values
			ESP.restart();
			break;
	}
}

// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
	CRGB::Red,
	CRGB::Gray, // 'white' is too bright compared to red and blue
	CRGB::Blue,
	CRGB::Black,

	CRGB::Red,
	CRGB::Gray,
	CRGB::Blue,
	CRGB::Black,

	CRGB::Red,
	CRGB::Red,
	CRGB::Gray,
	CRGB::Gray,
	CRGB::Blue,
	CRGB::Blue,
	CRGB::Black,
	CRGB::Black
};

byte setArraySidesPerTile(char* cNrOfSidesPerTile) {
	char cValue[40];
	byte beRetValue = 0;
	byte beIndex = 0;

	sprintf(cValue, "%s", cNrOfSidesPerTile);

	char *cNrOfSides = strtok(cValue, ",");

	while (cNrOfSides != NULL) {
		beNrOfSidesPerTile[beIndex++] = atoi(cNrOfSides);
		cNrOfSides = strtok(NULL, ",");
	}

	beRetValue = beIndex;

	return beRetValue;
}

// Standby mode
void standbyMode() {
	FastLED.clear();
	crgbLeds[1] = CRGB::Red;
	beLedBrightness = STANDBY_LED_BRIGHTNESS;
	FastLED.show();	
}

// -----------------------------------------------------------
