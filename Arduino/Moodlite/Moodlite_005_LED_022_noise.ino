// -----------------------------------------------------------
void noise16_1() {                                            // moves a noise up and down while slowly shifting to the side

  uint16_t scale = 1000;                                      // the "zoom factor" for the noise

  for (uint16_t i = 0; i < iNrOfLeds; i++) {

    uint16_t shift_x = beatsin8(5);                           // the x position of the noise field swings @ 17 bpm
    uint16_t shift_y = millis() / 100;                        // the y position becomes slowly incremented
    

    uint16_t real_x = (i + shift_x)*scale;                    // the x position of the noise field swings @ 17 bpm
    uint16_t real_y = (i + shift_y)*scale;                    // the y position becomes slowly incremented
    uint32_t real_z = millis() * 20;                          // the z position becomes quickly incremented
    
    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;   // get the noise data and scale it down

    uint8_t index = sin8(noise*3);                           // map LED color based on noise data
    uint8_t bri   = noise;

    crgbLeds[i] = ColorFromPalette(crgbCurrentPalette, index, bri, currentBlending);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }

    addEffect();

}

void noise16_1Tile(){
    uint16_t scale = 1000;                                      // the "zoom factor" for the noise
    int ledEnd = 0;
    int ledStart = 0;
    for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
        uint16_t shift_x = beatsin8(5);                           // the x position of the noise field swings @ 17 bpm
        uint16_t shift_y = millis() / 100;                        // the y position becomes slowly incremented
        uint16_t real_x = (i + shift_x)*scale;                    // the x position of the noise field swings @ 17 bpm
        uint16_t real_y = (i + shift_y)*scale;                    // the y position becomes slowly incremented
        uint32_t real_z = millis() * 20;                          // the z position becomes quickly incremented
        uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;   // get the noise data and scale it down
        uint8_t index = sin8(noise*3);                           // map LED color based on noise data
        uint8_t bri   = noise;
        uint8_t red = (millis() / 5) + (i * 12);                    // speed, length
        
        ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        for(int x = 0; x < (ledEnd - ledStart);x++){
            crgbLeds[ledStart + x] = ColorFromPalette(crgbCurrentPalette, index, bri, currentBlending);
        }
    }
    
    addEffect();
}
// -----------------------------------------------------------

// -----------------------------------------------------------
void noise16_2() {                                            // just moving along one axis = "lavalamp effect"

  uint8_t scale = 1000;                                       // the "zoom factor" for the noise

  for (uint16_t i = 0; i < iNrOfLeds; i++) {

    uint16_t shift_x = millis() / 10;                         // x as a function of time
    uint16_t shift_y = 0;

    uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
    uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
    uint32_t real_z = 4223;
    
    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;    // get the noise data and scale it down

    uint8_t index = sin8(noise*3);                            // map led color based on noise data
    uint8_t bri   = noise;

    crgbLeds[i] = ColorFromPalette(crgbCurrentPalette, index, bri, currentBlending);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.

  }

    addEffect();
  
}

void noise16_2Tile(){
    uint16_t scale = 1000;                                      // the "zoom factor" for the noise
    int ledEnd = 0;
    int ledStart = 0;
    for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
        uint16_t shift_x = millis() / 10;                         // x as a function of time
        uint16_t shift_y = 0;
        uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
        uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
        uint32_t real_z = 4223;
        uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;    // get the noise data and scale it down
        uint8_t index = sin8(noise*3);                            // map led color based on noise data
        uint8_t bri   = noise;
        
        ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        for(int x = 0; x < (ledEnd - ledStart);x++){
            crgbLeds[ledStart + x] = ColorFromPalette(crgbCurrentPalette, index, bri, currentBlending);
        }
    }
    
    addEffect();
}
// -----------------------------------------------------------

// -----------------------------------------------------------
void noise16_3() {                                            // no x/y shifting but scrolling along 

  uint8_t scale = 1000;                                       // the "zoom factor" for the noise

  for (uint16_t i = 0; i < iNrOfLeds; i++) {

    uint16_t shift_x = 4223;                                  // no movement along x and y
    uint16_t shift_y = 1234;

    uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
    uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
    uint32_t real_z = millis()*2;                             // increment z linear

    uint8_t noise = inoise16(real_x, real_y, real_z) >> 7;    // get the noise data and scale it down

    uint8_t index = sin8(noise*3);                            // map led color based on noise data
    uint8_t bri   = noise;

    crgbLeds[i] = ColorFromPalette(crgbCurrentPalette, index, bri, currentBlending);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }

    addEffect();
  
}

void noise16_3Tile(){
    uint16_t scale = 1000;                                      // the "zoom factor" for the noise
    int ledEnd = 0;
    int ledStart = 0;
    for (int i = 0; i < beNrOfSidesPerTileSize; i++) {
        uint16_t shift_x = 4223;                                  // no movement along x and y
        uint16_t shift_y = 1234;
        uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
        uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
        uint32_t real_z = millis()*2;                             // increment z linear
        uint8_t noise = inoise16(real_x, real_y, real_z) >> 7;    // get the noise data and scale it down
        uint8_t index = sin8(noise*3);                            // map led color based on noise data
        uint8_t bri   = noise;
        
        ledEnd = ledEnd + (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        ledStart = ledEnd - (beNrOfSidesPerTile[i] * beNrOfLedsCorner);
        for(int x = 0; x < (ledEnd - ledStart);x++){
            crgbLeds[ledStart + x] = ColorFromPalette(crgbCurrentPalette, index, bri, currentBlending);
        }
    }
    
    addEffect();
}
// -----------------------------------------------------------
