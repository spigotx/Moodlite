/*
	Name:     MoodLeaf.ino
	Created:  27.12.2018
	Version:  1.0
	Author:   Steve Wagg aka CdRsKuLL, and team
	Notes:    Web page: http://moodlite.co.uk 
		      Go to http://192.168.100.250 in a web browser connected to this access point to see it
*/


// --- Libraries ---
#include <FastLED.h>
#include <ESP8266WiFi.h>

// --- Debugging ---
#define DEBUGON false // flag to turn on/off debugging
#define Serial if(DEBUGON)Serial 

#if DEBUGON
#define DEBUG(x)    DEBUG(x)
#define DEBUGLN(x)   DEBUGLN(x)
#else
#define DEBUG(x)
#define DEBUGLN(x)
#endif

// --- Constants ---
// RGB Strip
#define LED_PIN     6

// How many Leds in strip
#define NUM_LEDS    48

#define BRIGHTNESS  250
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 30

// WiFi AP
// Replace with your network credentials
const char* WIFI_AP_PASSWORD = "Esp8266";

const char* WIFI_SSID     = "NetworkName";
const char* WIFI_PASSWORD = "NetworkPassword";

// If you want to set a static IP, then uncommment the three below lines AND the line WiFi.config(ip, gateway, subnet); in the void setup() below. Otherwise it will be allocated one via your DCHP server
//IPAddress ip(192, 168, 1, 40); //set static ip
//IPAddress gateway(192, 168, 1, 1); //set getteway
//IPAddress subnet(255, 255, 255, 0);//set subnet

// --- Variables ---
String sHeader;
String sPatattern = "1";
int iBrightPos1 = 0;
int iBrightPos2 = 0;
String sBrightString = String(5);
int iSpeedPos1 = 0;
int iSpeedPos2 = 0;
String sSpeedString = String(5);

CRGB crgbLeds[NUM_LEDS];
// Set web server port number to 80
WiFiServer wifiServer(80);

CRGBPalette16 crgbCurrentPalette;
TBlendType    currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void setup() {
	// --- Initialize serial communication ---
	Serial.begin(9600);
	delay( 3000 ); // power-up safety delay
	DEBUG("Connecting to ");
	DEBUGLN(WIFI_SSID);
  
	//WiFi.config(ip, gateway, subnet);  //uncomment if you want to use a static IP
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		DEBUG(".");
	}

  // Print local IP address and start web server
  DEBUGLN("");
  DEBUGLN("WiFi connected.");
  DEBUGLN("IP address: ");
  DEBUGLN(WiFi.localIP());
  wifiServer.begin();

  delay(500);
  DEBUG(".");
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(crgbLeds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  crgbCurrentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
}


void loop()
{
  WiFiClient wifiClient = wifiServer.available();   // Listen for incoming clients

  if (wifiClient) {                             // If a new client connects,
    DEBUGLN("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (wifiClient.connected()) {            // loop while the client's connected
      if (wifiClient.available()) {             // if there's bytes to read from the client,
        char c = wifiClient.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        sHeader += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            wifiClient.println("HTTP/1.1 200 OK");
            wifiClient.println("Content-type:text/html");
            wifiClient.println("Connection: close");
            wifiClient.println();

            // reads the url for pattern change
            if (sHeader.indexOf("GET /1/on") >= 0) {
              DEBUGLN("LED 1");
              sPatattern = "1";
              crgbCurrentPalette = RainbowColors_p; currentBlending = LINEARBLEND;
            } else if (sHeader.indexOf("GET /2/on") >= 0) {
              DEBUGLN("LED 2");
              sPatattern = "2";
              crgbCurrentPalette = RainbowStripeColors_p;  currentBlending = NOBLEND;
            } else if (sHeader.indexOf("GET /3/on") >= 0) {
              DEBUGLN("LED 3");
              sPatattern = "3";;
              crgbCurrentPalette = RainbowStripeColors_p; currentBlending = LINEARBLEND;
            } else if (sHeader.indexOf("GET /4/on") >= 0) {
              DEBUGLN("LED 4");
              sPatattern = "4";
              setupTotallyRandomPalette(); currentBlending = LINEARBLEND;
            } else if (sHeader.indexOf("GET /5/on") >= 0) {
              DEBUGLN("LED 5");
              sPatattern = "5";
              SetupPurpleAndGreenPalette(); currentBlending = LINEARBLEND;
            } else if (sHeader.indexOf("GET /6/on") >= 0) {
              DEBUGLN("LED 6");
              sPatattern = "6";
              crgbCurrentPalette = crgbCurrentPalette = CloudColors_p; currentBlending = LINEARBLEND;
            } else if (sHeader.indexOf("GET /7/on") >= 0) {
              DEBUGLN("LED 7");
              sPatattern = "7";
              crgbCurrentPalette = crgbCurrentPalette = PartyColors_p; currentBlending = LINEARBLEND;
            } else if (sHeader.indexOf("GET /8/on") >= 0) {
              DEBUGLN("LED 8");
              sPatattern = "8";
              crgbCurrentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND;
            }

            if (sHeader.indexOf("GET /?bvalue=") >= 0) {
              iBrightPos1 = sHeader.indexOf('=');
              iBrightPos2 = sHeader.indexOf('&');
              sBrightString = sHeader.substring(iBrightPos1 + 1, iBrightPos2);
              FastLED.setBrightness( sBrightString.toInt() );
              DEBUGLN(sBrightString);
            }

            if (sHeader.indexOf("GET /?svalue=") >= 0) {
              iSpeedPos1 = sHeader.indexOf('=');
              iSpeedPos2 = sHeader.indexOf('&');
              sSpeedString = sHeader.substring(iSpeedPos1 + 1, iSpeedPos2); //this variable is updated at the end of the void sub
              DEBUGLN(sSpeedString);
            }

            // Display the HTML web page
            wifiClient.println("<!DOCTYPE html><html>");
            wifiClient.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            wifiClient.println("<link rel=\"icon\" href=\"data:,\">");
            wifiClient.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            wifiClient.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 25px;");
            wifiClient.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            wifiClient.println(".textstyle { color: #FFFFFF;}");
            wifiClient.println(".slider { width: 300px; }</style></head>");
            wifiClient.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");

            // Web Page Heading
            wifiClient.println("<body style =\"background-color: #000000; color: #FFFFFF;\"><h1>moodlite</h1>");
            if (sPatattern == "1") {
              wifiClient.println("<hr><p class=\"textstyle\">PATTERN 1<br><br>");
            }
            if (sPatattern == "2") {
              wifiClient.println("<hr><p class=\"textstyle\">PATTERN 2<br><br>");
            }
            if (sPatattern == "3") {
              wifiClient.println("<hr><p class=\"textstyle\">PATTERN 3<br><br>");
            }
            if (sPatattern == "4") {
              wifiClient.println("<hr><p class=\"textstyle\">PATTERN 4<br><br>");
            }
            if (sPatattern == "5") {
              wifiClient.println("<hr><p class=\"textstyle\">PATTERN 5<br><br>");
            }
            if (sPatattern == "6") {
              wifiClient.println("<hr><p class=\"textstyle\">PATTERN 6<br><br>");
            }
            if (sPatattern == "7") {
              wifiClient.println("<hr><p class=\"textstyle\">PATTERN 7<br><br>");
            }
            if (sPatattern == "8") {
              wifiClient.println("<hr><p class=\"textstyle\">PATTERN 8<br><br>");
            }

            //Web Page buttons
            wifiClient.println("<a href=\"/1/on\"><button class=\"button\">P1</button></a>");
            wifiClient.println("<a href=\"/2/on\"><button class=\"button\">P2</button></a>");
            wifiClient.println("<a href=\"/3/on\"><button class=\"button\">P3</button></a>");
            wifiClient.println("<a href=\"/4/on\"><button class=\"button\">P4</button></a>");
            wifiClient.println("</p>");
            wifiClient.println("<p>");
            wifiClient.println("<a href=\"/5/on\"><button class=\"button\">P5</button></a>");
            wifiClient.println("<a href=\"/6/on\"><button class=\"button\">P6</button></a>");
            wifiClient.println("<a href=\"/7/on\"><button class=\"button\">P7</button></a>");
            wifiClient.println("<a href=\"/8/on\"><button class=\"button\">P8</button></a>");

            //Web page sliders
            wifiClient.println("</p><hr><p class=\"textstyle\">BRIGHTNESS</p>");
            wifiClient.println("<input type=\"range\" min=\"1\" max=\"255\" class=\"slider\" id=\"brightSlider\" onchange=\"bright(this.value)\" value=\"" + sBrightString + "\"/>");
            wifiClient.println("<script>var slider = document.getElementById(\"brightSlider\");");
            wifiClient.println("var brightP = document.getElementById(\"iBrightPos\"); brightP.innerHTML = slider.value;");
            wifiClient.println("slider.oninput = function() { slider.value = this.value; brightP.innerHTML = this.value; }");
            wifiClient.println("$.ajaxSetup({timeout:1000}); function bright(pos) { ");
            wifiClient.println("$.get(\"/?bvalue=\" + pos + \"&\"); {Connection: close};}</script>");

            wifiClient.println("</p><p class=\"textstyle\">SPEED</p>");
            wifiClient.println("<input type=\"range\" min=\"1\" max=\"499\" class=\"slider\" id=\"speedSlider\" onchange=\"speed(this.value)\" value=\"" + sSpeedString + "\"/>");
            wifiClient.println("<script>var slider = document.getElementById(\"speedSlider\");");
            wifiClient.println("var speedP = document.getElementById(\"iSpeedPos\"); speedP.innerHTML = slider.value;");
            wifiClient.println("slider.oninput = function() { slider.value = this.value; speedP.innerHTML = this.value; }");
            wifiClient.println("$.ajaxSetup({timeout:1000}); function speed(pos) { ");
            wifiClient.println("$.get(\"/?svalue=\" + pos + \"&\"); {Connection: close};}</script>");

            wifiClient.println("</body></html>");

            // The HTTP response ends with another blank line
            wifiClient.println();
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
    // Clear the sHeader variable
	  sHeader = "";
    // Close the connection
    wifiClient.stop();
    DEBUGLN("Client disconnected.");
    DEBUGLN("");
  }

  static uint8_t ui8StartIndex = 0;
  ui8StartIndex = ui8StartIndex + 1; /* motion speed */
  fillLedsFromPaletteColors( ui8StartIndex);
  FastLED.show();
  FastLED.delay(1000 / sSpeedString.toInt());
}

void fillLedsFromPaletteColors( uint8_t ui8ColorIndex)
{
  uint8_t ui8Brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    crgbLeds[i] = ColorFromPalette( crgbCurrentPalette, ui8ColorIndex, ui8Brightness, currentBlending);
    ui8ColorIndex += 3;
  }
}

void setupTotallyRandomPalette()
{
  for ( int i = 0; i < 16; i++) {
    crgbCurrentPalette[i] = CHSV( random8(), 255, random8());
  }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
/*
void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid( crgbCurrentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  crgbCurrentPalette[0] = CRGB::White;
  crgbCurrentPalette[4] = CRGB::White;
  crgbCurrentPalette[8] = CRGB::White;
  crgbCurrentPalette[12] = CRGB::White;

}
*/

// This function sets up a palette of crgbPurple and green stripes.
void SetupPurpleAndGreenPalette()
{
  CRGB crgbPurple = CHSV( HUE_PURPLE, 255, 255);
  CRGB crgbGreen  = CHSV( HUE_GREEN, 255, 255);
  CRGB crgbBlack  = CRGB::Black;

  crgbCurrentPalette = CRGBPalette16(
                     crgbGreen,  crgbGreen,  crgbBlack,  crgbBlack,
                     crgbPurple, crgbPurple , crgbBlack,  crgbBlack,
                     crgbGreen,  crgbGreen,  crgbBlack,  crgbBlack,
                     crgbPurple, crgbPurple, crgbBlack,  crgbBlack );
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
