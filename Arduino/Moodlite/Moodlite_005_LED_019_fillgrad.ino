// -----------------------------------------------------------
void fill_grad() {
  
  uint8_t starthue = beatsin8(5, 0, 255);
  uint8_t endhue = beatsin8(7, 0, 255);
  
  if (starthue < endhue) {
    fill_gradient(crgbLeds, iNrOfLeds, CHSV(starthue,255,255), CHSV(endhue,255,255), FORWARD_HUES);    // If we don't have this, the colour fill will flip around. 
  } else {
    fill_gradient(crgbLeds, iNrOfLeds, CHSV(starthue,255,255), CHSV(endhue,255,255), BACKWARD_HUES);
  }

    addEffect();
  
}
// -----------------------------------------------------------
