// -----------------------------------------------------------

void fillLedsFromPaletteColors(uint8_t ui8ColorIndex)
{
    uint8_t ui8Brightness = 255;

    for (int i = 0; i < ((iNrOfLeds / beNrOfLedsCorner) + beNrOfLedsCorner); i++) {
        for (int j = i * beNrOfLedsCorner; j < ((i * beNrOfLedsCorner) + beNrOfLedsCorner); j++) {
            crgbLeds[j] = ColorFromPalette(crgbCurrentPalette, ui8ColorIndex, ui8Brightness, currentBlending);
        }
        ui8ColorIndex += 3;
    }

    addEffect();
}

// -----------------------------------------------------------
