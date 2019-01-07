/*
	Name:     Moodlite.ino
	Created:  27.12.2018
	Version:  1.1.1
	AuthorS:  Steve Wagg aka CdRsKuLL, 
			  Spigot (M.V.)
	License:  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License v3.0 as published by the Free Software Foundation. 
			  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
			  See the GNU General Public License for more details.
			  You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/gpl.html>
	Notes:    Wether you use this project, have learned something from it, or just like it, please consider supporting us on our web site. :)
			  Web page: http://moodlite.co.uk
			  Go to http://192.168.100.250 in a web browser connected to this access point to see it
*/

// --- Libraries ---
// LED Strip
// Disable interrupts to avoid WS2812B strip flickering
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 1
#include "FastLED.h"
// ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
// EEPROM
#include <EEPROM.h>
// SPIFFS
#include <SPIFFSEditor.h>
// Time
#include <TimeLib.h>
#include <Timezone.h>
// Other
#include "Timer.h"
#include <base64.h>
#include <string>

// --- Debugging ---
#define DEBUGON false // flag to turn on/off debugging
#define Serial if (DEBUGON) Serial 

#if DEBUGON
	#define DEBUG(x)     Serial.print(x)
	#define DEBUGLN(x)   Serial.println(x)
#else
	#define DEBUG(x)
	#define DEBUGLN(x)
#endif

// --- Constants ---
// --- EEPROM data ---
#define SSID_START							0x0
#define SSID_MAX							0x20
#define PASSWORD_START						(SSID_START + SSID_MAX)
#define PASSWORD_MAX						0x40
#define URL_START							(PASSWORD_START + PASSWORD_MAX)
#define URL_MAX								0x50
#define HOSTNAME_START						(URL_START + URL_MAX)
#define HOSTNAME_MAX						 0x50
#define LED_BRIGHTNESS_START				(HOSTNAME_START + HOSTNAME_MAX)
#define LED_BRIGHTNESS_MAX					0x1
#define BACKLIGHT_START						(LED_BRIGHTNESS_START + LED_BRIGHTNESS_MAX)
#define BACKLIGHT_MAX						0x1
#define LED_COLOR_START						(BACKLIGHT_START + BACKLIGHT_MAX)
#define LED_COLOR_MAX						0x6
#define AUTDISPLAYON_START					(LED_COLOR_START + LED_COLOR_MAX)
#define AUTDISPLAYON_MAX					0x1
#define DISPLAYON_START						(AUTDISPLAYON_START + AUTDISPLAYON_MAX)
#define DISPLAYON_MAX						0x1
#define DISPLAYOFF_START					(DISPLAYON_START + DISPLAYON_MAX)
#define DISPLAYOFF_MAX						0x1
#define LED_SPEED_START						(DISPLAYOFF_START + DISPLAYOFF_MAX)
#define LED_SPEED_MAX						0x1
#define LED_PATTERN_START					(LED_SPEED_START + LED_SPEED_MAX)
#define LED_PATTERN_MAX						0x1
#define NUMBER_OF_LEDS_START				(LED_PATTERN_START + LED_PATTERN_MAX)
#define NUMBER_OF_LEDS_MAX					 0x3
#define NTP_SERVER_START  					(NUMBER_OF_LEDS_START + NUMBER_OF_LEDS_MAX)
#define NTP_SERVER_MAX      				0x50
#define NUMBER_OF_LEDS_CORNER_START  		(NTP_SERVER_START + NTP_SERVER_MAX)
#define NUMBER_OF_LEDS_CORNER_MAX		 	0x1

// RGB Strip
#define LED_PIN				 6

// How many Leds in strip
#define MAX_NUM_LEDS	   253
#define NUM_LEDS		   253

#define LED_TYPE			WS2812B
#define COLOR_ORDER			GRB

// Timers
// 30min
const int TR30M = 1800000;
// 1h
const int TR1H = 3600000;

//NTP server
// Ntp local port to listen for UDP packets
const unsigned int NTPLOCALUDPPORT = 2390;
// Ntp time stamp is in the first 48 bytes of the message
const int NTPPACKETSIZE = 48;

// WiFi AP
// AP Password
// ! Must be at least 8 chars long
const char* WIFI_AP_PASSWORD = "Moodlite";

// --- Variables ---
// WiFi settings
String sHostName = "Moodlite";
String sWifiSsd = "";
String sWifiPassword = "";

// RGB Strip
// Brightness
// Range: 0 - 255
// 0 - Off (Dark:Black)
// 128 - Half lit
// 255 - Fully lit
byte beLedBrightness;
byte beLedOldBrightness = 15;
String sBrightness;
boolean bBacklight = true;
boolean bAllowLedParamMod = false;
String sLedHexColor;
boolean bAutDisplayEnabled = true;
byte beDisplayOn;
byte beDisplayOff;
byte beRedLight = 255;
byte beGreenLight = 0;
byte beBlueLight = 0;
byte beLedSpeed = 10;
byte beLedOldSpeed;
byte beLedPattern = 0;
int iNrOfLeds;
String sNrOfLeds;
byte beNrOfLedsCorner;
String sNrOfLedsCorner;

// NTP server
// Server name
String sNtpServer;
// Buffer to hold incoming and outgoing packets
byte beNtpPacketBuffer[NTPPACKETSIZE];
// No Ntp packet received
boolean bNtpPacketError = false;
// Counter which triggers NTP server check
byte beNtpCounterTrigger = 0;

// --- Objects initialization ---
// Structure to store RGB colors converted from HEX color code
struct stRGBColors {
  byte beRed;
  byte beGreen;
  byte beBlue;
};
struct stRGBColors stLedColors;

TBlendType    currentBlending;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

// Initialize Wifi client
WiFiClient wifiClient;

// WiFi AP
IPAddress ipaWifiApIP(192, 168, 100, 250);
IPAddress ipaWifiApGateway(192, 168, 100, 1);
IPAddress ipaWifiApSubnet(255, 255, 255, 0);

// LEDs
CRGB crgbLeds[NUM_LEDS];
CRGB crgbLedColors = CRGB::White;
CRGB crgbOffColor = CRGB::Black;
CRGBPalette16 crgbCurrentPalette;

extern CRGBPalette16 myRedWhiteBluePalette;

// Timer
Timer trCheckNtpServer;

// UDP Packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

// Ntp Server
// Time zones
// Central European Time (Frankfurt, Paris)
// Central European Summer Time
TimeChangeRule tchrCest = { "CEST", Last, Sun, Mar, 2, 120 };
// Central European Standard Time
TimeChangeRule tchrCet = { "CET", Last, Sun, Oct, 3, 60 };
Timezone tzCe(tchrCest, tchrCet);
TimeChangeRule *tcr;

// Ntp IP address
IPAddress ipaNtpServerIp;

// Time
time_t utc, localTime;

// --- Asynchronous web server  ---
String sLastStatus = "";
String sStatusLog = "";

AsyncWebServer asyncWebServer(80);
AsyncWebSocket ayncWebSocket("/ws"); // access at ws://[esp ip]/ws

class ByteString : public String {
	public:
		ByteString(void *data, size_t len) :
			String() {
			copy(data, len);
		}

		ByteString() :
			String() {
		}

		String& copy(const void *data, unsigned int length) {
			if (!reserve(length)) {
			invalidate();
			return (*this);
			}
			len = length;
			memcpy(buffer, data, length);
			buffer[length] = 0;
			return (*this);
		}
};

// Asynchronous TCP Client to retrieve data/time
struct AsyncHTTPClient {
	AsyncClient *aClient = NULL;

	bool         initialized = false;
	String       protocol;
	String       base64Authorization;
	String       host;
	int          port;
	String       uri;
	String       request;

	ByteString   response;
	int          statusCode;
	void(*onSuccess)();
	void(*onFail)(String);

	void initialize(String url) {
		// check for : (http: or https:
		int index = url.indexOf(':');

		if (index < 0) {
			initialized = false;                   // This is not a URLs
		}

		protocol = url.substring(0, index);
		DEBUGLN(protocol);
		url.remove(0, (index + 3));           // remove http:// or https://

		index = url.indexOf('/');
		String hostPart = url.substring(0, index);
		DEBUGLN(hostPart);
		url.remove(0, index);           // remove hostPart part

		// get Authorization
		index = hostPart.indexOf('@');

		if (index >= 0) {
			// auth info
			String auth = hostPart.substring(0, index);
			hostPart.remove(0, index + 1);                // remove auth part including @
			base64Authorization = base64::encode(auth);
		}

		// get port
		port = 80;               //Default
		index = hostPart.indexOf(':');
		if (index >= 0) {
			host = hostPart.substring(0, index); // hostname
			host.remove(0, (index + 1));         // remove hostname + :
			DEBUGLN(host);
			port = host.toInt();                 // get port
			DEBUGLN(port);
		}
		else {
			host = hostPart;
			DEBUGLN(host);
		}
		uri = url;
		if (protocol != "http") {
			initialized = false;
		}

		DEBUGLN(initialized);
		request = "GET " + uri + " HTTP/1.1\r\nHost: " + host + "\r\n\r\n";

		DEBUGLN(request);
		initialized = true;
	}

	int getStatusCode() {
		return (statusCode);
	}

	String getBody() {
		if (statusCode == 200) {
			int bodyStart = response.indexOf("\r\n\r\n") + 4;
			return (response.substring(bodyStart));
		}
		else {
			return ("");
		}
	}

	static void clientError(void *arg, AsyncClient *client, int error) {
	DEBUGLN("Connect Error");
	AsyncHTTPClient *self = (AsyncHTTPClient *)arg;
	self->onFail("Connection error");
	self->aClient = NULL;
	delete client;
	}

	static void clientDisconnect(void *arg, AsyncClient *client) {
	DEBUGLN("Disconnected");
	AsyncHTTPClient *self = (AsyncHTTPClient *)arg;
	self->aClient = NULL;
	delete client;
	}

	static void clientData(void *arg, AsyncClient *client, void *data, size_t len) {
		DEBUGLN("Got response");

		AsyncHTTPClient *self = (AsyncHTTPClient *)arg;
		self->response = ByteString(data, len);
		String status = self->response.substring(9, 12);
		self->statusCode = atoi(status.c_str());
		DEBUGLN(status.c_str());

		if (self->statusCode == 200) {
			self->onSuccess();
		}
		else {
			self->onFail("Failed with code " + status);
		}
	}

	static void clientConnect(void *arg, AsyncClient *client) {
		DEBUGLN("Connected");

		AsyncHTTPClient *self = (AsyncHTTPClient *)arg;

		self->response.copy("", 0);
		self->statusCode = -1;

		// Clear oneError handler
		self->aClient->onError(NULL, NULL);

		// Set disconnect handler
		client->onDisconnect(clientDisconnect, self);

		client->onData(clientData, self);

		//send the request
		client->write(self->request.c_str());
	}

	void makeRequest(void(*success)(), void(*fail)(String msg)){
		onFail = fail;

		if (!initialized) {
			fail("Not initialized");
			return;
		}

		if (aClient) {           //client already exists
			fail("Call taking forever");
			return;
		}

		aClient = new AsyncClient();

		if (!aClient) {           //could not allocate client
			fail("Out of memory");
			return;
		}

		onSuccess = success;

		aClient->onError(clientError, this);

		aClient->onConnect(clientConnect, this);

		if (!aClient->connect(host.c_str(), port)) {
			DEBUGLN("Connect Fail");
			fail("Connection failed");
			AsyncClient *client = aClient;
			aClient = NULL;
			delete client;
		}
	}
};

AsyncHTTPClient httpClient;

// --- Setup functions ---
void setup() {
	EEPROM.begin(512);
	SPIFFS.begin();

	// --- Initialize serial communication ---
	Serial.begin(9600);

	// --- Uncomment to reset ---
	// Erase EEPROM
	//eraseEEPROM();
	// Erase SPIFFS
	//eraseSPIFFS();

	// --- Initialize EEPROM data ----
	// Load EEPROM data
	initEEPROMData();

	// --- Initialize WiFi ---
	initWiFi();

	// If Wifi connection is defined start the program
	// Else start only web server
	if (sWifiSsd.length() > 0) {
		// --- RGB LED Strip ---
		// Cahne default NUM_LEDS value with value stored in memory
		#ifdef NUM_LEDS
		#undef NUM_LEDS
		#endif
    
		#define NUM_LEDS iNrOfLeds
    
		// WS2812B - LED Type settings (to detect LED types use RGBCalibre sketch)
		// GRB - Postupnost farieb
		FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(crgbLeds, iNrOfLeds);
		//FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(new CRGB[iNrOfLeds], iNrOfLeds);
		// With setMaxRefreshRate, no need to use delay() after FastLED.Show()
		// The nr. means FPS and it sets ms for Fastled
		// 1000/FPS = ms
		// 1000/40 = 25ms -> FastLED.Show() than waits 25ms for each LED
		//FastLED.setMaxRefreshRate(40);
		FastLED.setBrightness(beLedBrightness);

		// Turn Off all LEDs
		FastLED.clear();

		// Start LED pattern
		changeLedPatternParameters(beLedPattern);

		// --- UDP ---
		udp.begin(NTPLOCALUDPPORT);

		// If NTP server is defined start checking time
		if (sNtpServer.length() > 0) {
		  // --- Initialize NTP Server ---
		  checkNtpServer((void *)0);
  
		  // --- Initialize Timers ---
		  // Check NTP Server - Update actual time
		  trCheckNtpServer.every(TR1H, checkNtpServer, (void*)0);
		}
	}

	// --- Web server ---
	MDNS.addService("http", "tcp", 80);

	asyncWebServer.on("/", HTTP_GET, mainHandler);
	asyncWebServer.on("/system", HTTP_GET, systemHandler);
	asyncWebServer.on("/set_wifi", HTTP_POST, wifiHandler);

	asyncWebServer.serveStatic("/css", SPIFFS, "/css");
	asyncWebServer.serveStatic("/js", SPIFFS, "/js");
	asyncWebServer.serveStatic("/images", SPIFFS, "/images");

	// Attach AsyncWebSocket
	ayncWebSocket.onEvent(onEvent);
	asyncWebServer.addHandler(&ayncWebSocket);

	asyncWebServer.begin();
	ayncWebSocket.enable(true);
}

// --- Main loop function ---
void loop()
{
	// Display LED pattern if: Turn light ON enabled, Nr. of LEDs per corner > 0, LED pattern is set
	if (checkDisplay() && beNrOfLedsCorner > 0 && beLedPattern != 0 && beLedPattern != 99) {
		static uint8_t ui8StartIndex = 0;
		ui8StartIndex += 1; /* motion speed */
		fillLedsFromPaletteColors(ui8StartIndex);
		FastLED.show();
		FastLED.delay(1000 / beLedSpeed);
	}

	// If NTP server is defined update timer
	if (sNtpServer.length() > 0)
		trCheckNtpServer.update();
}

// --- Functions ---
// Initialize WiFi
void initWiFi () {
	DEBUGLN("--- initWiFi - Start ---");
	// Disconnect() needs to be done befor connect again
	WiFi.disconnect();

	MDNS.begin(sHostName.c_str());

	// Wifi AP Config
	WiFi.mode(WIFI_AP_STA);

	WiFi.softAP(sHostName.c_str(), WIFI_AP_PASSWORD);
	WiFi.softAPConfig(ipaWifiApIP, ipaWifiApGateway, ipaWifiApSubnet);

	if (sWifiSsd.length() > 0) {
		// Set Host name
		WiFi.hostname(sHostName);

		if (sWifiPassword.length() > 0) {
			WiFi.begin(sWifiSsd.c_str(), sWifiPassword.c_str());
		}
		else {
			WiFi.begin(sWifiSsd.c_str());
		}

		delay(5000);
	}
	DEBUGLN("--- initWiFi - End ---");
}

// EEPROM functions
byte readByteFromEEPROM(int iStart) {
	int iPosition = iStart;
	byte beValue;

	EEPROM.get(iPosition, beValue);

	return (beValue);
}

// Write byte to EEPROM
void writeByteToEEPROM(byte beValue, int iStart) {
	int iPosition = iStart;

	//EEPROM.begin(512);
	EEPROM.write(iPosition, beValue);
	EEPROM.commit();
}

// Read string from EEPROM
String readStringFromEEPROM(int start, int max) {
	int    end = start + max;
	String s = "";

	for (int i = start; i < end; i++) {
		byte readByte = EEPROM.read(i);
		if (readByte > 0 && readByte < 128) {
			s += char(readByte);
		}
		else {
			break;
		}
	}
	return (s);
}

// Write string to EEPROM
void writeStringToEEPROM(String s, int start, int max) {
	int end = start + max;

	for (int i = start; i < end; i++) {
		if (i - start < s.length()) {
			EEPROM.write(i, s[i - start]);
		}
		else {
			EEPROM.write(i, 0);
			break;
		}
	}

	EEPROM.commit();
}

// Erase EEPROM
void eraseEEPROM() {
	DEBUGLN("--- eraseEEPROM - Start ---");

	for (int i = 0; i < EEPROM.length(); i++) {
		EEPROM.write(i, 0);
	}
	EEPROM.commit();

	DEBUGLN("--- eraseEEPROM - End ---");
}

// Erase SPIFFS
void eraseSPIFFS() {
	DEBUGLN("--- eraseSPIFFS - Start ---");

	SPIFFS.format();

	DEBUGLN("--- eraseSPIFFS - End ---");
}

// Initialize stored data
void initEEPROMData() {
	DEBUGLN("--- initEEPROMData - Start ---");

	// WiFi SSID
	sWifiSsd = readStringFromEEPROM(SSID_START, SSID_MAX);
	
	if (sWifiSsd.length() > 0) {
		// WiFi Password
		sWifiPassword = readStringFromEEPROM(PASSWORD_START, PASSWORD_MAX);

		// Hostname
		String sConfiguredHostname = readStringFromEEPROM(HOSTNAME_START, HOSTNAME_MAX);
		if (sConfiguredHostname.length() > 0) {
			sHostName = sConfiguredHostname;
		}

		// Number of LEDs per corner
		sNrOfLedsCorner = readByteFromEEPROM(NUMBER_OF_LEDS_CORNER_START);
		beNrOfLedsCorner = sNrOfLedsCorner.toInt();

		// Number of LEDs
		sNrOfLeds = readStringFromEEPROM(NUMBER_OF_LEDS_START, NUMBER_OF_LEDS_MAX);
		iNrOfLeds = sNrOfLeds.toInt();

		// LED Color
		sLedHexColor = readStringFromEEPROM(LED_COLOR_START, LED_COLOR_MAX);
		stLedColors = hexToRGB(sLedHexColor);
		crgbLedColors = CRGB(stLedColors.beRed, stLedColors.beGreen, stLedColors.beBlue);

		// Backlight
		bBacklight = readByteFromEEPROM(BACKLIGHT_START);

		// Display On/Off
		bAutDisplayEnabled = readByteFromEEPROM(AUTDISPLAYON_START);
		beDisplayOn = readByteFromEEPROM(DISPLAYON_START);
		beDisplayOff = readByteFromEEPROM(DISPLAYOFF_START);

		// LED Brightness
		beLedBrightness = readByteFromEEPROM(LED_BRIGHTNESS_START);

		// LED Speed
		beLedSpeed = readByteFromEEPROM(LED_SPEED_START);

		//LED Patterm
		beLedPattern = readByteFromEEPROM(LED_PATTERN_START);

		// NTP Server
		sNtpServer = readStringFromEEPROM(NTP_SERVER_START, NTP_SERVER_MAX);


		DEBUG("Wifi pass: ");
		DEBUGLN(sWifiPassword);
		DEBUG("Wifi hostname: ");
		DEBUGLN(sHostName);
		DEBUG("Backlight: ");
		DEBUGLN(bBacklight);
		DEBUG("Auto Turn on/off: ");
		DEBUGLN(bAutDisplayEnabled);
		DEBUG("Display On: ");
		DEBUGLN(beDisplayOn);
		DEBUG("Display Off: ");
		DEBUGLN(beDisplayOff);
		DEBUG("Nr. of LEDs: ");
		DEBUGLN(iNrOfLeds);
		DEBUG("Nr. of LEDs per corner: ");
		DEBUGLN(beNrOfLedsCorner);
		DEBUG("Brightness: ");
		DEBUGLN(beLedBrightness);
		DEBUG("Speed: ");
		DEBUGLN(beLedSpeed);
		DEBUG("Pattern: ");
		DEBUGLN(beLedPattern);
		DEBUG("LED color: ");
		DEBUGLN(sLedHexColor);
		DEBUG("NTP Server: ");
		DEBUGLN(sNtpServer);
	}

	DEBUG("Wifi ssid: ");
	DEBUGLN(sWifiSsd);

	DEBUGLN("--- initEEPROMData - End ---");
}

// Set default values to EEPROM
void setDefaulValues() {
  writeByteToEEPROM(1, BACKLIGHT_START);
  writeByteToEEPROM(20, LED_BRIGHTNESS_START);
  writeStringToEEPROM("FF0000", LED_COLOR_START, LED_COLOR_MAX);
  writeByteToEEPROM(1, NUMBER_OF_LEDS_CORNER_START);
  writeStringToEEPROM("3", NUMBER_OF_LEDS_START, NUMBER_OF_LEDS_MAX);
  writeByteToEEPROM(50, LED_SPEED_START);
}

// Web server functions
/*
  msg is
  <num>: for a message with no arguments or
  <num>:<string>:<value> for a message that has a key/value pair

*/
void handleWSMsg(AsyncWebSocketClient *client, char *msg) {
	DEBUGLN("--- handleWSMsg - Start ---");
	DEBUGLN(msg);

	String wholeMsg(msg);

	int code = wholeMsg.substring(0, wholeMsg.indexOf(':')).toInt();

	switch (code) {
		case 1:
			sendWifiValues(client);
			break;

		case 2:
			sendSettingsValues(client);
			break;

		case 3:
			sendLedValues(client);
			break;

		case 5:
			updateValue(wholeMsg.substring(wholeMsg.indexOf(':') + 1));
			break;
	}

	DEBUGLN("--- handleWSMsg - End ---");
}

void sendStatus(String msg) {
	DEBUGLN("--- sendStatus - Start ---");

	String json = "{\"type\":\"sv.status\",\"value\":\"" + msg + "\"}";

	ayncWebSocket.textAll(json);

	DEBUGLN("--- sendStatus - End ---");
}

void sendWifiValues(AsyncWebSocketClient *client) {
	DEBUGLN("--- sendWifiValues - Start ---");

	String json = String("{\"type\":\"sv.init.wifi\",\"value\":{\"ssid\":\"");

	json += sWifiSsd;
	json += "\",\"password\":\"";
	json += sWifiPassword;
	json += "\",\"hostname\":\"";
	json += sHostName;
	json += "\"}}";

	client->text(json);

	json = "{\"type\":\"sv.status\",\"value\":\"" + sLastStatus + "\"}";

	client->text(json);

	DEBUGLN("--- sendWifiValues - End ---");
}

void sendSettingsValues(AsyncWebSocketClient *client) {
	DEBUGLN("--- sendSettingsValues - Start ---");

	String url  = readStringFromEEPROM(URL_START, URL_MAX);
	String json = String("{\"type\":\"sv.init.settings\",\"value\":{") +
				"\"aut_display_enabled\":" + bAutDisplayEnabled + "," +
				"\"display_on\":" + beDisplayOn + "," +
				"\"display_off\":" + beDisplayOff + "," +
				"\"ntp_server\":\"" + sNtpServer + "\"," +   
				"\"led_strip_nr_of_leds_corner\":" + beNrOfLedsCorner + "," +
				"\"led_strip_nr_of_leds\":" + iNrOfLeds +
				"}}";
	
	client->text(json);
	
	DEBUGLN("--- sendSettingsValues - End ---");
}

void sendLedValues(AsyncWebSocketClient *client) {
	DEBUGLN("--- sendLedValues - Start ---");

	String json = String("{\"type\":\"sv.init.leds\",\"value\":{") +
				"\"backlight\":" + bBacklight + "," +
				"\"allow_led_params_mod\":" + bAllowLedParamMod + "," +
				"\"colorpickerfield_led_strip\":\"" + sLedHexColor + "\"," +
				"\"led_strip_brightness\":" + beLedBrightness + "," +
				"\"led_strip_speed\":" + beLedSpeed +
				"}}";

	client->text(json);
	
	DEBUGLN("--- sendLedValues - End ---");
}

void updateValue(String pair) {
	DEBUGLN("--- sendLedValues - updateValue ---");

	int index = pair.indexOf(':');

	// _key has to hang around because key points to an internal data structure
	String      _key  = pair.substring(0, index);
	const char *key   = _key.c_str();
	String      value = pair.substring(index + 1);

	// Automatic Display On/Off
	// True = On
	// False = Off
	if (strcmp("aut_display_enabled", key) == 0) {
		DEBUG("aut_display_enabled: ");
		DEBUGLN(value);

		// Convert string value "true"/"false" to boolean for further calculation
		const char *to_find = value.c_str(); // string we want to find

		if (strcmp (to_find, "true") == 0) { // this is the key: a match returns 0
		  bAutDisplayEnabled = true;
		}
		else {
		  bAutDisplayEnabled = false;
		}

		if (bAutDisplayEnabled) {
		  writeByteToEEPROM(1, AUTDISPLAYON_START);
		}
		else {
		  writeByteToEEPROM(0, AUTDISPLAYON_START);
		}
	}

	// Display On Hour
	// Hour when backlight is on
	else if (strcmp("display_on", key) == 0) {
		DEBUG("display_on: ");
		DEBUGLN(value);

		beDisplayOn = value.toInt();

		// Store value
		writeByteToEEPROM(beDisplayOn, DISPLAYON_START);
	}

	// Hour when backlight is off
	// Display Off Hour
	else if (strcmp("display_off", key) == 0) {
		DEBUG("display_off: ");
		DEBUGLN(value);

		beDisplayOff = value.toInt();

		// Store value
		writeByteToEEPROM(beDisplayOff, DISPLAYOFF_START);
	}

	// Backlight
	// True = On
	// False = Off
	else if (strcmp("backlight", key) == 0) {
		DEBUG("backlight: ");
		DEBUGLN(value);

		// Convert string value "true"/"false" to boolean for further calculation
		const char *to_find = value.c_str(); // string we want to find

		if (strcmp (to_find, "true") == 0) { // this is the key: a match returns 0
			bBacklight = true;
		}
		else {
			bBacklight = false;
		}

		if (bBacklight) {
			writeByteToEEPROM(1, BACKLIGHT_START);
		}
		else {
			writeByteToEEPROM(0, BACKLIGHT_START);
		}

		// If automatic backlight is turned Off
		if (!bAutDisplayEnabled) {
			// if backlight is On set brightness to last value
			if (bBacklight) {
				beLedBrightness = readByteFromEEPROM(LED_BRIGHTNESS_START);
			}
			// if backlight is Off set brightness to 0
			else {
				beLedBrightness = 0;
			}
			// Change Rgb Led brightness
			FastLED.setBrightness(beLedBrightness);
			FastLED.show();
		}

	}

	// Parameters modifications
	// True = On
	// False = Off
	// If On allow change colors and brightness
	else if (strcmp("allow_led_params_mod", key) == 0) {
		DEBUG("allow_led_params_mod: ");
		DEBUGLN(value);

		// Convert string value "true"/"false" to boolean for further calculation
		const char *to_find = value.c_str(); // string we want to find

		if (strcmp(to_find, "true") == 0) { // this is the key: a match returns 0
			bAllowLedParamMod = true;
		}
		else {
			bAllowLedParamMod = false;
		}

		// Store values if parametrization is disable, disabled write changes
		if (!bAllowLedParamMod) {
			DEBUGLN("Store data");
			DEBUGLN(beLedBrightness);
			writeByteToEEPROM(beLedBrightness, LED_BRIGHTNESS_START);
			DEBUGLN(sLedHexColor);
			writeStringToEEPROM(sLedHexColor, LED_COLOR_START, LED_COLOR_MAX);
			DEBUGLN(beLedSpeed);
			writeByteToEEPROM(beLedSpeed, LED_SPEED_START);
		}
	}

	// Clock's color
	// Color HEX value
	else if (strcmp("colorpickerfield_led_strip", key) == 0) {
		// If mods are enabled allow changes
		if (bAllowLedParamMod) {
			DEBUG("colorpickerfield_led_strip: ");
			DEBUGLN(value);

			sLedHexColor = value;

			stLedColors = hexToRGB(sLedHexColor);

			DEBUG("beRed: ");
			DEBUGLN(stLedColors.beRed);
			DEBUG("beGreen: ");
			DEBUGLN(stLedColors.beGreen);
			DEBUG("beBlue: ");
			DEBUGLN(stLedColors.beBlue);

			crgbLedColors = CRGB(stLedColors.beRed, stLedColors.beGreen, stLedColors.beBlue);

			// Change clock's color
			// Turn Off all LEDs
			FastLED.clear();

			if (beLedPattern == 0) {
			setLedsColor(0, iNrOfLeds, crgbLedColors);
			FastLED.show();
			}
		}
	}

	// Brightness
	// Led strip brightness value
	else if (strcmp("led_strip_brightness", key) == 0) {
		// If mods are enabled allow changes
		if (bAllowLedParamMod) {
			DEBUG("led_strip_brightness: ");
			DEBUGLN(value);

			beLedBrightness = value.toInt();

			// If brightness change is > 20 store brightness value
			if (abs(beLedBrightness - beLedOldBrightness) > 20) {
				beLedOldBrightness = beLedBrightness;
			}
			// Change Rgb Led brightness
			FastLED.setBrightness(beLedBrightness);
			FastLED.show();
			DEBUGLN(beLedBrightness);
		}
	}

	// Speed
	// Led strip speed value
	else if (strcmp("led_strip_speed", key) == 0) {
		// If mods are enabled allow changes
		if (bAllowLedParamMod) {
			DEBUG("led_strip_speed: ");
			DEBUGLN(value);

			beLedSpeed = value.toInt();

			// If speed change is > 10 store speed value
			if (abs(beLedSpeed - beLedOldSpeed) > 10) {
			beLedOldSpeed = beLedSpeed;
			}

			DEBUGLN(beLedSpeed);
		}
	}

	// LED pattern
	else if (strcmp("pattern", key) == 0) {
		DEBUG("pattern: ");
		DEBUGLN(value);

		beLedPattern = value.toInt();

		FastLED.clear();

		changeLedPatternParameters(beLedPattern);

		// Store value
		writeByteToEEPROM(beLedPattern, LED_PATTERN_START);

		DEBUGLN(beLedPattern);
	}

	// Set NTP server
	else if (strcmp("setNtpServer", key) == 0) {
	DEBUG("setNtpServer: ");
	DEBUGLN(value);

	sNtpServer = value;

	// Store value
	writeStringToEEPROM(sNtpServer, NTP_SERVER_START, NTP_SERVER_MAX);

	DEBUGLN(sNtpServer);

	// Stop the code and restart ESP
	beLedPattern = 99;
	changeLedPatternParameters(beLedPattern);
	}

	// Set number of LEDs per corner
	else if (strcmp("setNrOfLedsCorner", key) == 0) {
	DEBUG("setNrOfLedsCorner: ");
	DEBUGLN(value);

	sNrOfLedsCorner = value;
	beNrOfLedsCorner = value.toInt();

	// Store value
	writeByteToEEPROM(beNrOfLedsCorner, NUMBER_OF_LEDS_CORNER_START);

	DEBUGLN(beNrOfLedsCorner);

	// Stop the code and restart ESP
	beLedPattern = 99;
	changeLedPatternParameters(beNrOfLedsCorner);
	}

	// Set total number of LEDs
	else if (strcmp("setNrOfLeds", key) == 0) {
		DEBUG("setNrOfLeds: ");
		DEBUGLN(value);

		sNrOfLeds = value;
		iNrOfLeds = value.toInt();
		
		// Store value
		writeStringToEEPROM(sNrOfLeds, NUMBER_OF_LEDS_START, NUMBER_OF_LEDS_MAX);

		DEBUGLN(iNrOfLeds);

		// Stop the code and restart ESP
		beLedPattern = 99;
		changeLedPatternParameters(beLedPattern);    
	}

	DEBUGLN("--- sendLedValues - End ---");
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
	//Handle WebSocket event
	switch (type) {
		case WS_EVT_CONNECT:
		  DEBUGLN("WS connected");
		  break;

		case WS_EVT_DISCONNECT:
		  DEBUGLN("WS disconnected");
		  break;

		case WS_EVT_ERROR:
		  DEBUGLN("WS error");
		  DEBUGLN((char *)data);
		  break;

		case WS_EVT_PONG:
		  DEBUGLN("WS pong");
		  break;

		case WS_EVT_DATA:            // Yay we got something!
		  DEBUGLN("WS data");
		  AwsFrameInfo *info = (AwsFrameInfo *)arg;
		  if (info->final && info->index == 0 && info->len == len) {
			//the whole message is in a single frame and we got all of it's data
			if (info->opcode == WS_TEXT) {
			  DEBUGLN("WS text data");
			  data[len] = 0;
			  handleWSMsg(client, (char *)data);
			}
			else {
			  DEBUGLN("WS binary data");
			}
		  }
		  else {
			DEBUGLN("WS data was split up!");
		  }
		  break;
	}
}

String getInput(String type, String name, String label, String value) {
	String input = "<input type=\"" + type + "\" name=\"" + name + "\" placeholder=\"" + label + "\" value=\"" + value + "\"/>";

	return (input);
}

// Web server Handler
void mainHandler(AsyncWebServerRequest *request) {
	DEBUGLN("--- mainHandler - Start ---");
	

	// When no wifi connection is defined (main_ap.html from folder data is not used)
	// When settings submitted function wifiHandler() is called to store new credentials
	if (WiFi.status() != WL_CONNECTED) {
		request->send(SPIFFS, "/main_ap.html");
	}
	// When normal operation (index.html from folder data is used)
	else {
		request->send(SPIFFS, "/index.html");
	}

	DEBUGLN("--- mainHandler - End ---");
}

// Get the header for a 2 column table
String getTableHead2Col(String tableHeader, String col1Header, String col2Header) {
	String tableHead = "<h3>";

	tableHead += tableHeader;
	tableHead += "</h3>";
	tableHead += "<table><thead><tr><th>";
	tableHead += col1Header;
	tableHead += "</th><th>";
	tableHead += col2Header;
	tableHead += "</th></tr></thead><tbody>";

	return (tableHead);
}

String getTableRow2Col(String col1Val, String col2Val) {
	String tableRow = "<tr><td>";

	tableRow += col1Val;
	tableRow += "</td><td>";
	tableRow += col2Val;
	tableRow += "</td></tr>";

	return (tableRow);
}

String getTableRow2Col(String col1Val, int col2Val) {
	String tableRow = "<tr><td>";

	tableRow += col1Val;
	tableRow += "</td><td>";
	tableRow += col2Val;
	tableRow += "</td></tr>";

	return (tableRow);
}

String getTableFoot() {
	return ("</tbody></table></div></div>");
}

void systemHandler(AsyncWebServerRequest *request) {
	DEBUGLN("--- systemHandler - Start ---");

	String response("<html><head><title>stLixieClock Stats</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"stylesheet\" href=\"/assets/table.css\"/></head><body>");

	float voltage = (float)ESP.getVcc() / (float)1024;
	voltage -= 0.01f;       // by default reads high
	char dtostrfbuffer[15];
	dtostrf(voltage, 7, 2, dtostrfbuffer);
	String vccString = String(dtostrfbuffer);

	// ESP8266 Info table
	response += getTableHead2Col("ESP8266 information", "Name", "Value");
	response += getTableRow2Col("Sketch size", ESP.getSketchSize());
	response += getTableRow2Col("Free sketch size", ESP.getFreeSketchSpace());
	response += getTableRow2Col("Free heap", ESP.getFreeHeap());
	response += getTableRow2Col("Boot version", ESP.getBootVersion());
	response += getTableRow2Col("CPU Freqency (MHz)", ESP.getCpuFreqMHz());
	response += getTableRow2Col("SDK version", ESP.getSdkVersion());
	response += getTableRow2Col("Chip ID", ESP.getChipId());
	response += getTableRow2Col("Flash Chip ID", ESP.getFlashChipId());
	response += getTableRow2Col("Flash size", ESP.getFlashChipRealSize());
	response += getTableRow2Col("Vcc", vccString);
	response += getTableFoot();

	response += "</body></html>";
	request->send(200, "text/html", response);
	
	DEBUGLN("--- systemHandler - End ---");
}

// Called when changing wifi settings
// 1. when to Wifi connection defined
// 2. during normal operation
void wifiHandler(AsyncWebServerRequest *request) {
	DEBUGLN("--- wifiHandler - Start ---");

	int args = request->args();

	for (int i = 0; i < args; i++) {
		DEBUGLN(request->argName(i).c_str());
		DEBUGLN(request->arg(i).c_str());
	}

	AsyncWebParameter *p = request->getParam("ssid", true);
	if (p) {
		String _ssid = p->value();
		writeStringToEEPROM(_ssid, SSID_START, SSID_MAX);
		DEBUGLN(_ssid.c_str());
	}
	else {
		writeStringToEEPROM("", SSID_START, SSID_MAX);
	}

	p = request->getParam("password", true);
	if (p) {
		String _password = p->value();
		writeStringToEEPROM(_password, PASSWORD_START, PASSWORD_MAX);
		DEBUGLN(_password.c_str());
	}
	else {
		writeStringToEEPROM("", PASSWORD_START, PASSWORD_MAX);
	}

	p = request->getParam("hostname", true);
	if (p) {
		String _hostname = p->value();
		writeStringToEEPROM(_hostname, HOSTNAME_START, HOSTNAME_MAX);
		DEBUGLN(_hostname.c_str());
	}
	else {
		writeStringToEEPROM("", HOSTNAME_START, HOSTNAME_MAX);
	}

	DEBUGLN("--- wifiHandler - End ---");

  // Set default values
  DEBUGLN("--- setDefaulValues - Start ---");
  setDefaulValues();
  DEBUGLN("--- setDefaulValues - End ---");

	ESP.restart();
}


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

// Convert HEX color to RGB
struct stRGBColors hexToRGB(String sHexColor) {
	struct stRGBColors rgbColors;
	char charbuf[8];
	long int rgb;

	sHexColor.toCharArray(charbuf, 8);

	rgb = strtol(charbuf, 0, 16); //=>rgb=0x001234FE;

	rgbColors.beRed = (byte)(rgb >> 16);
	rgbColors.beGreen = (byte)(rgb >> 8);
	rgbColors.beBlue = (byte)(rgb);

	return rgbColors;
}

// Change LED pattern parameters
void changeLedPatternParameters(byte bePatternParameter) {
	switch (beLedPattern) {
		case 0:
			setLedsColor(0, iNrOfLeds, crgbLedColors);
			FastLED.show();
			break;

		case 1:
			crgbCurrentPalette = RainbowColors_p;
			currentBlending = LINEARBLEND;
			break;

		case 2:
			crgbCurrentPalette = RainbowStripeColors_p;
			currentBlending = NOBLEND;
			break;

		case 3:
			crgbCurrentPalette = RainbowStripeColors_p;
			currentBlending = LINEARBLEND;
			break;

		case 4:
			setupTotallyRandomPalette();
			currentBlending = LINEARBLEND;
			break;

		case 5:
			SetupPurpleAndGreenPalette();
			currentBlending = LINEARBLEND;
			break;

		case 6:
			crgbCurrentPalette = crgbCurrentPalette = CloudColors_p;
			currentBlending = LINEARBLEND;
			break;

		case 7:
			crgbCurrentPalette = crgbCurrentPalette = PartyColors_p;
			currentBlending = LINEARBLEND;
			break;

		case 8:
			crgbCurrentPalette = myRedWhiteBluePalette_p;
			currentBlending = LINEARBLEND;
			break;

		case 99:
			FastLED.clear();
			turnOffLeds(0, iNrOfLeds);
			FastLED.show();

			// Restart ESP to update values
			ESP.restart();
			break;
	}
}

// Check if actual hour is within display limits (Display On/Off hours)
boolean checkHour() {
	byte    beActualHour = hour();
	boolean bDisplayDigits = beActualHour >= beDisplayOn && beActualHour < beDisplayOff;
	boolean bRet = false;
  
	if (beDisplayOn >= beDisplayOff) {
		bDisplayDigits = beActualHour >= beDisplayOn || beActualHour < beDisplayOff;
	}

	if (bDisplayDigits) {
		bRet = true;
	}
  
	return(bRet);  
}

// Check if LEDs should be On/Off
boolean checkDisplay() {
	boolean bRet = false;

	// Turn LED On/Off
	// On = Automatic display is ON and Actual hour is within Display On limits
	//      OR
	//      Automatic display is ON and backlight (manual mode) is on
	// Off = everything else 
	if ((bAutDisplayEnabled && checkHour()) || (!bAutDisplayEnabled && bBacklight)) {
		// If parametrization is disabled read data from memory else use actual settings
		if (!bAllowLedParamMod) {
			beLedBrightness = readByteFromEEPROM(LED_BRIGHTNESS_START);
		}
		bRet = true;
	}
	else {
		beLedBrightness = 0;
	}
	// Execute only if device is configured
	if (sWifiSsd.length() > 0) {
		// Change Rgb Led brightness
		FastLED.setBrightness(beLedBrightness);
		FastLED.show();
	}

	return(bRet);
}

// NTP Server
void checkNtpServer(void *context) {
	DEBUGLN("--- checkNtpServer - Start ---");
  
	int cbParsePacket, i;

	cbParsePacket = 0;
	i = 1;

	// Send an NTP packet to a time server
	// Wait to see if a reply is available
	while (!cbParsePacket) {
		DEBUGLN(sNtpServer);
		DEBUGLN(ipaNtpServerIp);

		// Translate DNS name to IP
		WiFi.hostByName(sNtpServer.c_str(), ipaNtpServerIp);
      
		sendNTPpacket(ipaNtpServerIp);

		delay(1000 * i);

		cbParsePacket = udp.parsePacket();
		DEBUGLN("No packet yet");

		// i > 10 After 66s = (1000*i) = max 66s, stop sending packet and mark NTP packet error
		if (i > 10) {
			bNtpPacketError = true;
			break;
		}

		i++;
	}

	// Switch to another NTP Server
	if (!cbParsePacket) {
		DEBUGLN("No packet yet");
		DEBUGLN("Calling checkNtpServer");
		checkNtpServer((void *)0);
	}
	else {
		DEBUGLN(sNtpServer);
		DEBUGLN(ipaNtpServerIp);
		DEBUG("Packet received, length=");
		DEBUGLN(cbParsePacket);

		// Received a packet, read the data from it
		// read the packet into the buffer
		udp.read(beNtpPacketBuffer, NTPPACKETSIZE);

		//the timestamp starts at byte 40 of the received packet and is four bytes,
		// or two words, long. First, esxtract the two words:
		unsigned long highWord = word(beNtpPacketBuffer[40], beNtpPacketBuffer[41]);
		unsigned long lowWord = word(beNtpPacketBuffer[42], beNtpPacketBuffer[43]);

		// combine the four bytes (two words) into a long integer
		// this is NTP time (seconds since Jan 1 1900):
		unsigned long secsSince1900 = highWord << 16 | lowWord;

		// now convert NTP time into everyday time:
		// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
		const unsigned long seventyYears = 2208988800UL;

		// subtract seventy years:
		unsigned long epoch = secsSince1900 - seventyYears;

		// print Unix time:
		//DEBUGLN(epoch);

		utc = epoch;
		localTime = tzCe.toLocal(utc, &tcr);

		// Set device time
		setTime(localTime);

		DEBUGLN(hour());
		DEBUGLN(minute());
		DEBUGLN(second());
	}
  
	DEBUGLN("--- checkNtpServer - End ---");
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
	DEBUGLN("--- sendNTPpacket - Start ---");

	// set all bytes in the buffer to 0
	memset(beNtpPacketBuffer, 0, NTPPACKETSIZE);

	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	beNtpPacketBuffer[0] = 0b11100011;      // LI, Version, Mode
	beNtpPacketBuffer[1] = 0;               // Stratum, or type of clock
	beNtpPacketBuffer[2] = 6;               // Polling Interval
	beNtpPacketBuffer[3] = 0xEC;            // Peer Clock Precision
											// 8 bytes of zero for Root Delay & Root Dispersion
	beNtpPacketBuffer[12] = 49;
	beNtpPacketBuffer[13] = 0x4E;
	beNtpPacketBuffer[14] = 49;
	beNtpPacketBuffer[15] = 52;

	// All NTP fields have been given values, now
	// You can send a packet requesting a timestamp:
	udp.beginPacket(address, 123); //NTP requests are to port 123
	udp.write(beNtpPacketBuffer, NTPPACKETSIZE);
	udp.endPacket();

	DEBUGLN("--- sendNTPpacket - End ---");
}

// LED patterns
void fillLedsFromPaletteColors( uint8_t ui8ColorIndex)
{
	uint8_t ui8Brightness = 255;

	for ( int i = 0; i < ((iNrOfLeds / beNrOfLedsCorner) + beNrOfLedsCorner); i++) {
		for (int j = i * beNrOfLedsCorner; j < ((i * beNrOfLedsCorner) + beNrOfLedsCorner); j++) {
			crgbLeds[j] = ColorFromPalette(crgbCurrentPalette, ui8ColorIndex, ui8Brightness, currentBlending);
		}
		ui8ColorIndex += 3;
	}
}

void setupTotallyRandomPalette()
{
	for ( int i = 0; i < 16; i++) {
		crgbCurrentPalette[i] = CHSV( random8(), 255, random8());
	}
}

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
							crgbPurple, crgbPurple, crgbBlack,  crgbBlack 
							);
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
