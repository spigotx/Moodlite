// -----------------------------------------------------------

CRGB clr1;
CRGB clr2;
uint8_t speed;
uint8_t loc1;

void blendwave(){
    speed = beatsin8(6, 0, 255);
    
    clr1 = blend(CHSV(beatsin8(3, 0, 255), 255, 255), CHSV(beatsin8(4, 0, 255), 255, 255), speed);
    clr2 = blend(CHSV(beatsin8(4, 0, 255), 255, 255), CHSV(beatsin8(3, 0, 255), 255, 255), speed);
    
    loc1 = beatsin8(10, 0, iNrOfLeds - 1);
    
    fill_gradient_RGB(crgbLeds, 0, clr2, loc1, clr1);
    fill_gradient_RGB(crgbLeds, loc1, clr2, iNrOfLeds - 1, clr1);

    addEffect();
}

// -----------------------------------------------------------
