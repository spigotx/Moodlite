// Downloaded from http://moodlite.co.uk  - Open source
// Written by Steve Wagg aka CdRsKuLL
// Release v1.0 27th Dec 2018

// Load librarys
#include <FastLED.h>
#include <ESP8266WiFi.h>

// Replace with your network credentials
const char* ssid     = "NetworkName";
const char* password = "NetworkPassword";

// If you want to set a static IP, then uncommment the three below lines AND the line WiFi.config(ip, gateway, subnet); in the void setup() below. Otherwise it will be allocated one via your DCHP server
//IPAddress ip(192, 168, 1, 40); //set static ip
//IPAddress gateway(192, 168, 1, 1); //set getteway
//IPAddress subnet(255, 255, 255, 0);//set subnet

// Set the pin you are outputting on and the number of LEDs (5 tiles = 15 LEDs). 
#define LED_PIN     6
#define NUM_LEDS    48

String header;
String pat = "1";
int brightpos1 = 0;
int brightpos2 = 0;
String brightString = String(5);
int speedpos1 = 0;
int speedpos2 = 0;
String speedString = String(5);
#define BRIGHTNESS  250
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
// Set web server port number to 80
WiFiServer server(80);
#define UPDATES_PER_SECOND 30
CRGBPalette16 currentPalette;
TBlendType    currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void setup() {
  Serial.begin(115200);
  delay( 3000 ); // power-up safety delay
  Serial.print("Connecting to ");
  Serial.println(ssid);
  //WiFi.config(ip, gateway, subnet);  //uncomment if you want to use a static IP
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  delay(500);
  Serial.print(".");
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
}


void loop()
{
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();




            // reads the url for pattern change
            if (header.indexOf("GET /1/on") >= 0) {
              Serial.println("LED 1");
              pat = "1";
              currentPalette = RainbowColors_p; currentBlending = LINEARBLEND;
            } else if (header.indexOf("GET /2/on") >= 0) {
              Serial.println("LED 2");
              pat = "2";
              currentPalette = RainbowStripeColors_p;  currentBlending = NOBLEND;
            } else if (header.indexOf("GET /3/on") >= 0) {
              Serial.println("LED 3");
              pat = "3";;
              currentPalette = RainbowStripeColors_p; currentBlending = LINEARBLEND;
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("LED 4");
              pat = "4";
              SetupTotallyRandomPalette(); currentBlending = LINEARBLEND;
            } else if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("LED 5");
              pat = "5";
              SetupPurpleAndGreenPalette(); currentBlending = LINEARBLEND;
            } else if (header.indexOf("GET /6/on") >= 0) {
              Serial.println("LED 6");
              pat = "6";
              currentPalette = currentPalette = CloudColors_p; currentBlending = LINEARBLEND;
            } else if (header.indexOf("GET /7/on") >= 0) {
              Serial.println("LED 7");
              pat = "7";
              currentPalette = currentPalette = PartyColors_p; currentBlending = LINEARBLEND;
            } else if (header.indexOf("GET /8/on") >= 0) {
              Serial.println("LED 8");
              pat = "8";
              currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND;
            }

            if (header.indexOf("GET /?bvalue=") >= 0) {
              brightpos1 = header.indexOf('=');
              brightpos2 = header.indexOf('&');
              brightString = header.substring(brightpos1 + 1, brightpos2);
              FastLED.setBrightness( brightString.toInt() );
              Serial.println(brightString);
            }

            if (header.indexOf("GET /?svalue=") >= 0) {
              speedpos1 = header.indexOf('=');
              speedpos2 = header.indexOf('&');
              speedString = header.substring(speedpos1 + 1, speedpos2); //this variable is updated at the end of the void sub
              Serial.println(speedString);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 25px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".textstyle { color: #FFFFFF;}");
            client.println(".slider { width: 300px; }</style></head>");
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");

            // Web Page Heading
            client.println("<body style =\"background-color: #000000; color: #FFFFFF;\"><h1>moodlite</h1>");
            if (pat == "1") {
              client.println("<hr><p class=\"textstyle\">PATTERN 1<br><br>");
            }
            if (pat == "2") {
              client.println("<hr><p class=\"textstyle\">PATTERN 2<br><br>");
            }
            if (pat == "3") {
              client.println("<hr><p class=\"textstyle\">PATTERN 3<br><br>");
            }
            if (pat == "4") {
              client.println("<hr><p class=\"textstyle\">PATTERN 4<br><br>");
            }
            if (pat == "5") {
              client.println("<hr><p class=\"textstyle\">PATTERN 5<br><br>");
            }
            if (pat == "6") {
              client.println("<hr><p class=\"textstyle\">PATTERN 6<br><br>");
            }
            if (pat == "7") {
              client.println("<hr><p class=\"textstyle\">PATTERN 7<br><br>");
            }
            if (pat == "8") {
              client.println("<hr><p class=\"textstyle\">PATTERN 8<br><br>");
            }

            //Web Page buttons
            client.println("<a href=\"/1/on\"><button class=\"button\">P1</button></a>");
            client.println("<a href=\"/2/on\"><button class=\"button\">P2</button></a>");
            client.println("<a href=\"/3/on\"><button class=\"button\">P3</button></a>");
            client.println("<a href=\"/4/on\"><button class=\"button\">P4</button></a>");
            client.println("</p>");
            client.println("<p>");
            client.println("<a href=\"/5/on\"><button class=\"button\">P5</button></a>");
            client.println("<a href=\"/6/on\"><button class=\"button\">P6</button></a>");
            client.println("<a href=\"/7/on\"><button class=\"button\">P7</button></a>");
            client.println("<a href=\"/8/on\"><button class=\"button\">P8</button></a>");

            //Web page sliders
            client.println("</p><hr><p class=\"textstyle\">BRIGHTNESS</p>");
            client.println("<input type=\"range\" min=\"1\" max=\"255\" class=\"slider\" id=\"brightSlider\" onchange=\"bright(this.value)\" value=\"" + brightString + "\"/>");
            client.println("<script>var slider = document.getElementById(\"brightSlider\");");
            client.println("var brightP = document.getElementById(\"brightPos\"); brightP.innerHTML = slider.value;");
            client.println("slider.oninput = function() { slider.value = this.value; brightP.innerHTML = this.value; }");
            client.println("$.ajaxSetup({timeout:1000}); function bright(pos) { ");
            client.println("$.get(\"/?bvalue=\" + pos + \"&\"); {Connection: close};}</script>");

            client.println("</p><p class=\"textstyle\">SPEED</p>");
            client.println("<input type=\"range\" min=\"1\" max=\"499\" class=\"slider\" id=\"speedSlider\" onchange=\"speed(this.value)\" value=\"" + speedString + "\"/>");
            client.println("<script>var slider = document.getElementById(\"speedSlider\");");
            client.println("var speedP = document.getElementById(\"speedPos\"); speedP.innerHTML = slider.value;");
            client.println("slider.oninput = function() { slider.value = this.value; speedP.innerHTML = this.value; }");
            client.println("$.ajaxSetup({timeout:1000}); function speed(pos) { ");
            client.println("$.get(\"/?svalue=\" + pos + \"&\"); {Connection: close};}</script>");

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  FillLEDsFromPaletteColors( startIndex);
  FastLED.show();
  FastLED.delay(1000 / speedString.toInt());
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 3;
  }
}

void SetupTotallyRandomPalette()
{
  for ( int i = 0; i < 16; i++) {
    currentPalette[i] = CHSV( random8(), 255, random8());
  }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid( currentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CRGB::White;
  currentPalette[4] = CRGB::White;
  currentPalette[8] = CRGB::White;
  currentPalette[12] = CRGB::White;

}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
  CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  CRGB green  = CHSV( HUE_GREEN, 255, 255);
  CRGB black  = CRGB::Black;

  currentPalette = CRGBPalette16(
                     green,  green,  black,  black,
                     purple, purple, black,  black,
                     green,  green,  black,  black,
                     purple, purple, black,  black );
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

