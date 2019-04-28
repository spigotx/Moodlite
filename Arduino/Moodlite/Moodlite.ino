/*
	Name:     Moodlite.ino
	Created:  27.12.2018
	Version:  3.0
	AuthorS:  Spigot (M.V.), Shiryou (A.R.N.) & Steve Wagg aka CdRsKuLL
	License:  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License v3.0 as published by the Free Software Foundation.
			  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
			  See the GNU General Public License for more details.
			  You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/gpl.html>
	Support:  Wether you use this project, have learned something from it, or just like it, please consider supporting us on our web site. :)
			  Web page: http://moodlite.co.uk
	Notes:    Go to http://192.168.100.250 in a web browser connected to this access point to see it
*/

// ------------------ Basic settings ------------------
#define FASTLED_INTERRUPT_RETRY_COUNT	20			// Use this to determine how many times FastLED will attempt to re-transmit a frame if interrupted
#define STANDBY_LED_BRIGHTNESS			2			// Define Standby mode LED brightness
#define DEBUG_ON						false		// Flag to turn On/Off debugging		[True = On, False = Off]
#define STATIC_IP						false		// Flag to use static IP				[True = Static IP, False = DHCP - AP mode]
#define RESET_BUTTON					false		// Flag to use hardware reset button	[True = Use HW button, False = Don't use HW button]
#define NTP_ON							false		// Flag to use NTP Server				[True = Use NTP server, False = Don't use NTP server]
#define MQTT_ON							false		// Flag to use MQTT						[True = Use MQTT server, False = Don't use MQTT server]
#define OTA_ON							true		// Flag to use OTA updates				[True = Use OTA updates, False = Don't use OTA updates]
#define PIR_ON							false		// Flag to use PIR sensor				[True = Use PIR sensor, False = Don't use PIR sensor]
#define ESP8266							true		// Flag to use ESP8266					
//#define ESP32							true		// Flag to use ESP32					
// ----------------------------------------------------

// ------------------- Libraries ----------------------
// LED Strip
#include "FastLED.h"

#if ESP8266									// ESP8266
	#include <ESP8266WiFi.h>
	#include <ESP8266mDNS.h>
	#include <ESPAsyncTCP.h>
	#include <ESPAsyncWebServer.h>
#elif ESP32									// ESP32
	#include <WiFi.h>
	#include <ESPmDNS.h>
	#include <AsyncTCP.h>
	#include <ESPAsyncWebServer.h>
	#include "SPIFFS.h"
	#define FORMAT_SPIFFS_IF_FAILED true
#endif

// EEPROM
#include <EEPROM.h>
// SPIFFS
#include <SPIFFSEditor.h>
// Time
#include <TimeLib.h>
#include <Timezone.h>
// MQTT
#include <PubSubClient.h>
// OTA
#include <ArduinoOTA.h>
// Other
#include "Timer.h"
#include <base64.h>
#include <string>
// ----------------------------------------------------

// -------------------- Program -----------------------
// --- Debugging ---
#define Serial if (DEBUG_ON) Serial 

#if DEBUG_ON
	#define DEBUG(x)		Serial.print(x)
	#define DEBUGLN(x)		Serial.println(x)
	#define DEBUGF(x, y)	Serial.printf(x, y)
#else
	#define DEBUG(x)
	#define DEBUGLN(x)
#endif

// --- WiFi IP Settings ---
#if STATIC_IP
	// Static IP
	IPAddress ipaWifiIP(192, 168, 100, 250);
	IPAddress ipaWifiSubnet(255, 255, 255, 0);
	IPAddress ipaWifiGateway(192, 168, 100, 1);
	// If you don't wish to use DNS leave "ipaWifiDns" variable as it is, otherwise set yours DNS or Google DNS (8.8.8.8)
	IPAddress ipaWifiDns(0, 0, 0, 0);
	// Configure:
	// cHostName, cWifiSsid, cWifiPassword are 20 chars long, the last char is ending of char array
	char cHostName[21] = "Moodlite";
	char cWifiSsid[21] = "....";
	char cWifiPassword[21] = "....";
#else
	// WiFi AP (DHCP)
	IPAddress ipaWifiApIP(192, 168, 100, 250);
	IPAddress ipaWifiApSubnet(255, 255, 255, 0);
	IPAddress ipaWifiApGateway(192, 168, 100, 1);
	// Leave unconfigured:
	// cHostName, cWifiSsid, cWifiPassword are 20 chars long, the last char is ending of char array
	char cHostName[21] = "Moodlite";
	char cWifiSsid[21] = "";
	char cWifiPassword[21] = "";
	// AP Password
	// ! Must be at least 8 chars long
	const char* WIFI_AP_PASSWORD = "Moodlite";
#endif

// --- Reset button ---
#if RESET_BUTTON
	#define RST_BTN_PIN		D6
#endif

// --- RGB LED ---
#define LED_PIN				6      // NodeMCU PIN	
//#define LED_PIN				D5      // Wemos PIN D1

// LEDs settings
#define MAX_NUM_LEDS		253
#define NUM_LEDS			253

#define LED_TYPE			WS2812B
#define COLOR_ORDER			GRB

// --- PIR sensor ---
#if PIR_ON
	#define PIR_PIN			D7	
#endif

// --- EEPROM data ---
#define SSID_START								0x0
#define SSID_MAX								0x15
#define PASSWORD_START							(SSID_START + SSID_MAX)
#define PASSWORD_MAX							0x29
#define HOSTNAME_START							(PASSWORD_START + PASSWORD_MAX)
#define HOSTNAME_MAX							0x15
#define LED_BRIGHTNESS_START					(HOSTNAME_START + HOSTNAME_MAX)
#define LED_BRIGHTNESS_MAX						0x1
#define BACKLIGHT_START							(LED_BRIGHTNESS_START + LED_BRIGHTNESS_MAX)
#define BACKLIGHT_MAX							0x1
#define LED_COLOR_START							(BACKLIGHT_START + BACKLIGHT_MAX)
#define LED_COLOR_MAX							0x6
#define AUTDISPLAYON_START						(LED_COLOR_START + LED_COLOR_MAX)
#define AUTDISPLAYON_MAX						0x1
#define DISPLAYON_START							(AUTDISPLAYON_START + AUTDISPLAYON_MAX)
#define DISPLAYON_MAX							0x1
#define DISPLAYOFF_START						(DISPLAYON_START + DISPLAYON_MAX)
#define DISPLAYOFF_MAX							0x1
#define LED_SPEED_START							(DISPLAYOFF_START + DISPLAYOFF_MAX)
#define LED_SPEED_MAX							0x1
#define LED_PATTERN_START						(LED_SPEED_START + LED_SPEED_MAX)
#define LED_PATTERN_MAX							0x1
#define NUMBER_OF_LEDS_START					(LED_PATTERN_START + LED_PATTERN_MAX)
#define NUMBER_OF_LEDS_MAX						0x3
#define NTP_SERVER_START						(NUMBER_OF_LEDS_START + NUMBER_OF_LEDS_MAX)
#define NTP_SERVER_MAX							0x20
#define NUMBER_OF_LEDS_CORNER_START				(NTP_SERVER_START + NTP_SERVER_MAX)
#define NUMBER_OF_LEDS_CORNER_MAX				0x1
#define MQTT_HOSTNAME_START						(NUMBER_OF_LEDS_CORNER_START + NUMBER_OF_LEDS_CORNER_MAX)
#define MQTT_HOSTNAME_MAX						0x15
#define MQTT_PORT_START							(MQTT_HOSTNAME_START + MQTT_HOSTNAME_MAX)
#define MQTT_PORT_MAX							0x0A
#define MQTT_USERNAME_START						(MQTT_PORT_START + MQTT_PORT_MAX)
#define MQTT_USERNAME_MAX						0x15
#define MQTT_USERPASSWORD_START					(MQTT_USERNAME_START + MQTT_USERNAME_MAX)
#define MQTT_USERPASSWORD_MAX					0x15
#define MQTT_CLIENT_ID_START					(MQTT_USERPASSWORD_START + MQTT_USERPASSWORD_MAX)
#define MQTT_CLIENT_ID_MAX						0x15
#define PIR_SENSOR_ENABLED_START				(MQTT_CLIENT_ID_START + MQTT_CLIENT_ID_MAX)
#define PIR_SENSOR_ENABLED_MAX					0x1
#define LED_COLOR_PATTERN_START					(PIR_SENSOR_ENABLED_START + PIR_SENSOR_ENABLED_MAX)
#define LED_COLOR_PATTERN_MAX					0x1
#define NUMBER_OF_SIDES_PER_TILE_START			(LED_COLOR_PATTERN_START + LED_COLOR_PATTERN_MAX)
#define NUMBER_OF_SIDES_PER_TILE_MAX			0x29
#define PIR_SENSOR_DISPLAY_ON_TIME_START		(NUMBER_OF_SIDES_PER_TILE_START + NUMBER_OF_SIDES_PER_TILE_MAX)
#define PIR_SENSOR_DISPLAY_ON_TIME_MAX			0x1
#define LED_EFFECT_START                        (PIR_SENSOR_DISPLAY_ON_TIME_START + PIR_SENSOR_DISPLAY_ON_TIME_MAX)
#define LED_EFFECT_MAX                          0x5
#define TIMEZONE_DST_ABBREV_START				(LED_EFFECT_START + LED_EFFECT_MAX)
#define TIMEZONE_DST_ABBREV_MAX					0x6
#define TIMEZONE_DST_WEEK_START					(TIMEZONE_DST_ABBREV_START + TIMEZONE_DST_ABBREV_MAX)
#define TIMEZONE_DST_WEEK_MAX					0x1
#define TIMEZONE_DST_DOW_START					(TIMEZONE_DST_WEEK_START + TIMEZONE_DST_WEEK_MAX)
#define TIMEZONE_DST_DOW_MAX					0x1
#define TIMEZONE_DST_MONTH_START				(TIMEZONE_DST_DOW_START + TIMEZONE_DST_DOW_MAX)
#define TIMEZONE_DST_MONTH_MAX					0x1
#define TIMEZONE_DST_HOUR_START					(TIMEZONE_DST_MONTH_START + TIMEZONE_DST_MONTH_MAX)
#define TIMEZONE_DST_HOUR_MAX					0x1
#define TIMEZONE_DST_OFFSET_START				(TIMEZONE_DST_HOUR_START + TIMEZONE_DST_HOUR_MAX)
#define TIMEZONE_DST_OFFSET_MAX					0x6
#define TIMEZONE_STD_ABBREV_START				(TIMEZONE_DST_OFFSET_START + TIMEZONE_DST_OFFSET_MAX)
#define TIMEZONE_STD_ABBREV_MAX					0x6
#define TIMEZONE_STD_WEEK_START					(TIMEZONE_STD_ABBREV_START + TIMEZONE_STD_ABBREV_MAX)
#define TIMEZONE_STD_WEEK_MAX					0x1
#define TIMEZONE_STD_DOW_START					(TIMEZONE_STD_WEEK_START + TIMEZONE_STD_WEEK_MAX)
#define TIMEZONE_STD_DOW_MAX					0x1
#define TIMEZONE_STD_MONTH_START				(TIMEZONE_STD_DOW_START + TIMEZONE_STD_DOW_MAX)
#define TIMEZONE_STD_MONTH_MAX					0x1
#define TIMEZONE_STD_HOUR_START					(TIMEZONE_STD_MONTH_START + TIMEZONE_STD_MONTH_MAX)
#define TIMEZONE_STD_HOUR_MAX					0x1
#define TIMEZONE_STD_OFFSET_START				(TIMEZONE_STD_HOUR_START + TIMEZONE_STD_HOUR_MAX)
#define TIMEZONE_STD_OFFSET_MAX					0x6
#define DISPLAY_MODE_START						(TIMEZONE_STD_OFFSET_START + TIMEZONE_STD_OFFSET_MAX)
#define DISPLAY_MODE_MAX						0x1

// --- Constants ---
// Timers
// 1s
const int TR1S = 1000;
// 5s
const int TR5S = 5000;
// 10s
const int TR10S = 10000;
// 1m
const int TR1M = 60000;
// 5min
const int TR5M = 300000;
// 15min
const int TR15M = 900000;
// 30min
const int TR30M = 1800000;
// 1h
const int TR1H = 3600000;
// 3h
const int TR3H = 10800000;

// EEPROM
const int EEPROM_MIN_ADDR = 0;
const int EEPROM_MAX_ADDR = 511;

//NTP server
// Ntp local port to listen for UDP packets
const unsigned int NTPLOCALUDPPORT = 2390;
// Ntp time stamp is in the first 48 bytes of the message
const int NTPPACKETSIZE = 48;

// --- Variables ---
// RGB Strip
// Brightness
// Range: 0 - 255
// 0 - Off (Dark:Black)
// 128 - Half lit
// 255 - Fully lit
byte beLedBrightness;
byte beLedOldBrightness;
boolean bBrighntessChange;
boolean bBacklight = true;
String sLedHexColor;
boolean bDisplayMode = false;
boolean bAutDisplayEnabled = true;
byte beDisplayOn;
byte beDisplayOff;
byte beRedLight = 255;
byte beGreenLight = 0;
byte beBlueLight = 0;
byte beLedSpeed = 10;
byte beLedOldSpeed;
byte beLedPattern = 0;
byte beLedColorPattern = 0;
int iLedEffect = 0;
char cLedEffect[5];
int iNrOfLeds;
String sNrOfLeds;
byte beNrOfLedsCorner;
String sNrOfLedsCorner;
byte beNrOfSidesPerTile[20];
byte beNrOfSidesPerTileSize;
char cNrOfSidesPerTile[40];

// NTP server
// Server name
String sNtpServer;
// Buffer to hold incoming and outgoing packets
byte beNtpPacketBuffer[NTPPACKETSIZE];
// No Ntp packet received
boolean bNtpPacketError = false;
// Counter which triggers NTP server check
byte beNtpCounterTrigger = 0;
char cTzDstRuleTz[7];
char cTzDstRuleWeek[2];
byte beTzDstRuleWeek;
char cTzDstRuleDow[2];
byte beTzDstRuleDow;
char cTzDstRuleMonth[3];
byte beTzDstRuleMonth;
char cTzDstRuleHour[3];
byte beTzDstRuleHour;
char cTzDstRuleOffset[6];
int iTzDstRuleOffset;
char cTzStdRuleTz[7];
char cTzStdRuleWeek[2];
byte beTzStdRuleWeek;
char cTzStdRuleDow[2];
byte beTzStdRuleDow;
char cTzStdRuleMonth[3];
byte beTzStdRuleMonth;
char cTzStdRuleHour[3];
byte beTzStdRuleHour;
char cTzStdRuleOffset[6];
int iTzStdRuleOffset;

// Button
boolean bResetButtonState = 0;

// MQTT
char cMqttHostName[21] = "";
char cMqttPort[11] = "";
int iMqttPort;
char cMqttUserName[21] = "";
char cMqttUserPassword[21] = "";
char cMqttClientId[21] = "";

// Topics for MQTT client to SET values to Moodlite
const char* cMqttSetTopic[14] = { "Moodlite/Brightness/Set", "Moodlite/Pattern/Set", "Moodlite/FixedPattern/Set", "Moodlite/PatternColor/Set", "Moodlite/Color/Set", "Moodlite/Speed/Set", "Moodlite/Backlight/Set", "Moodlite/DisplayOnEnabled/Set",
								"Moodlite/DisplayOn/Set", "Moodlite/DisplayOff/Set", "Moodlite/PirSensorEndabled/Set", "Moodlite/PirSensorDisplayOnTime/Set", "Moodlite/Effect/Set", "Moodlite/DisplayMode/Set"
								};
// Topics for MQTT client to GET values from Moodlite
const char* cMqttGetTopic[14] = { "Moodlite/Brightness/Get", "Moodlite/Pattern/Get", "Moodlite/FixedPattern/Get", "Moodlite/PatternColor/Get", "Moodlite/Color/Get", "Moodlite/Speed/Get", "Moodlite/Backlight/Get", "Moodlite/DisplayOnEnabled/Get",
								"Moodlite/DisplayOn/Get", "Moodlite/DisplayOff/Get", "Moodlite/PirSensorEndabled/Get", "Moodlite/PirSensorDisplayOnTime/Get", "Moodlite/Effect/Get", "Moodlite/DisplayMode/Get"
								};

// String sMqttValue = "";
boolean bMqttConnError = false;

// PIR sensor
// The time the sensor calibrates (10s - 60s)
int iPirCalibrationTime = TR10S;
// The time when the sensor outputs a HIGH impulse
int iPirHighIn;
// The time when the sensor outputs a LOW impulse
int iPirLowIn;
// The amount of milliseconds the sensor has to be LOW before we assume all motion has stopped
int iPirPause = TR5S;
// The amount of milliseconds the sensor has to be HIGH before we turn of all LEDs
byte bePirDisplayOnTime = 1;

boolean bPirLockLow = true;
boolean bPirLockHigh = true;
boolean bPirTakeLowTime;
boolean bPirDisplayOn = false;
boolean bPirSensorEnabled = false;

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

// LEDs
CRGB crgbLeds[NUM_LEDS];
CRGB crgbLedColors = CRGB::White;
CRGB crgbOffColor = CRGB::Black;
CRGBPalette16 crgbCurrentPalette;

extern CRGBPalette16 myRedWhiteBluePalette;

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void(*SimplePatternList[])();
SimplePatternList gPatterns = { &fullColor, &standard, &confetti, &sinelon, &juggle, &bpm, &kitt, &plasma, &blendwave, &inoise8_fire, &rainbow_beat, &rainbow, &rainbowWithGlitter, &standardTile, &beatwave, &blur, &fill_grad, &inoise_8, &mover, &noise16_1, &noise16_2, &noise16_3, &three_sin, &confettiTile, &sinelonTile, &juggleTile, &bpmTile, &kittTile, &plasmaTile, &beatwaveTile, &inoise_8Tile, &moverTile, &noise16_1Tile, &noise16_2Tile, &noise16_3Tile, &three_sinTile};
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
static int16_t iRndINoise;

// NTP server
// Time zones
TimeChangeRule tchrDst = { "UTC", First, Sun, Jan, 1, 0 };
TimeChangeRule tchrStd = { "UTC", First, Sun, Jan, 1, 0 };
Timezone tz(tchrDst, tchrStd);
TimeChangeRule *tcr;

// Timer
Timer trNtpCheckServer;
Timer trMqttCheckMessages;
Timer trMqttSendStatus;

// UDP Packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

// Ntp IP address
IPAddress ipaNtpServerIp;
// Mqtt IP address
IPAddress ipaMqttServerIp;

// Time
time_t utc, localTime;

// Mqtt
PubSubClient mqttClient(wifiClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// --- Asynchronous web server  ---
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
			initialized = false;					// This is not a URLs
		}

		protocol = url.substring(0, index);
		DEBUGLN(protocol);
		url.remove(0, (index + 3));					// remove http:// or https://

		index = url.indexOf('/');
		String hostPart = url.substring(0, index);
		DEBUGLN(hostPart);
		url.remove(0, index);						// remove hostPart part

		// get Authorization
		index = hostPart.indexOf('@');

		if (index >= 0) {
			// auth info
			String auth = hostPart.substring(0, index);
			hostPart.remove(0, index + 1);                // remove auth part including @
			base64Authorization = base64::encode(auth);
		}

		// get port
		port = 80;								 //Default
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

	void makeRequest(void(*success)(), void(*fail)(String msg)) {
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
	// --- Initialize serial communication ---
	Serial.begin(9600);

	// --- Initialize EEPROM ---
	EEPROM.begin(512);

	// --- Initialize SPIFFS ---
	#if ESP8266
		SPIFFS.begin();
	#elif ESP32
		SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED);
	#endif

	// --- Initialize Push button ---
	// Initialize reset buton pin
	#if RESET_BUTTON
		pinMode(RST_BTN_PIN, INPUT);
	#endif

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
	if (strlen(cWifiSsid) > 0) {
		// --- RGB LED Strip ---
		// Change default NUM_LEDS value with value stored in memory
		#ifdef NUM_LEDS
			#undef NUM_LEDS
		#endif

		#define NUM_LEDS iNrOfLeds

		FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(crgbLeds, iNrOfLeds);
		FastLED.setBrightness(beLedBrightness);

		// Turn Off all LEDs
		FastLED.clear();
		FastLED.show();

		// Start LED pattern
        changeLedColorPatternParameters(beLedColorPattern);
		switchLedPattern(beLedPattern);

		// --- Initialize NTP Server ---
		#if NTP_ON
			// --- UDP ---
			udp.begin(NTPLOCALUDPPORT);

			// If NTP server is defined start checking time
			if (sNtpServer.length() > 0) {
				// --- Initialize NTP Server ---
				ntpCheckServer((void *)0);

				// --- Initialize Timer ---
				// Check NTP Server - Update actual time
				trNtpCheckServer.every(TR3H, ntpCheckServer, (void*)0);
			}
		#endif	

		// --- Initialize MQTT connection ---
		#if MQTT_ON
			// If MQTT server is defined start connection
			if (strlen(cMqttHostName) > 0) {
				// --- Initialize Timer ---
				// Check for MQTT mesages
				trMqttCheckMessages.every(500, mqttCheckMessage, (void*)0);
				// Send MQTT status
				trMqttSendStatus.every(TR5M, mqttSendStatus, (void*)0);

				// --- Connect to MQTT broker ---
				// Translate Name to IP
				WiFi.hostByName(cMqttHostName, ipaMqttServerIp);
				mqttClient.setServer(ipaMqttServerIp, iMqttPort);
				mqttClient.setCallback(mqttCallback);
			}
		#endif	

		// --- Initialize OTA ---
		#if OTA_ON
			ArduinoOTA.onStart([]() {
				String sType;

				if (ArduinoOTA.getCommand() == U_FLASH) {
					sType = "sketch";
				}
				else { // U_SPIFFS
					sType = "filesystem";
				}

				SPIFFS.end();

				DEBUGLN("Start updating " + sType);
			});

			ArduinoOTA.onEnd([]() {
				DEBUGLN("End");
				ESP.restart();
			});

			ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
				DEBUGF("Progress: %u%%\r", (progress / (total / 100)));
			});

			ArduinoOTA.onError([](ota_error_t error) {
				DEBUGF("Error[%u]: ", error);

				if (error == OTA_AUTH_ERROR) {
					DEBUGLN("Authentication error");
				}
				else if (error == OTA_BEGIN_ERROR) {
					DEBUGLN("Begin error");
				}
				else if (error == OTA_CONNECT_ERROR) {
					DEBUGLN("Connection error");
				}
				else if (error == OTA_RECEIVE_ERROR) {
					DEBUGLN("Receive error");
				}
				else if (error == OTA_END_ERROR) {
					DEBUGLN("End error");
				}
			});

			ArduinoOTA.begin();
		#endif	

		// --- Initialize PIR sensor ---
		#if PIR_ON
			pinMode(PIR_PIN, INPUT);
			digitalWrite(PIR_PIN, LOW);

			// Sensor time to calibrate
			DEBUG("Calibrating sensor ");

			for (int i = 0; i < (iPirCalibrationTime / 1000); i++) {
				DEBUG(".");
				delay(1000);
			}

			DEBUGLN(" done.");
			DEBUGLN("PIR sensor activated.");
		#endif
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
	// If OTA is not defined skip code in compiler
	#if OTA_ON
		ArduinoOTA.handle();
	#endif

	// If RESET_BUTTON is not defined skip code in compiler
	#if RESET_BUTTON
		bResetButtonState = digitalRead(RST_BTN_PIN);

		// When reset button pressed, erase EEPROM and restart ESP
		if (bResetButtonState) {
			eraseEEPROM();
		}
	#endif

	// If NTP is not defined skip code in compiler
	#if NTP_ON
		// If NTP server is defined update timer
		if (sNtpServer.length() > 0)
			trNtpCheckServer.update();
	#endif	

	// If MQTT is not defined skip code in compiler
	#if MQTT_ON
		// If MQTTS server is defined and no connection error exists, connect to server and check for message
		if (strlen(cMqttHostName) > 0 && !(bMqttConnError)) {
			if (!mqttClient.connected()) {
				mqttConnect();
			}

			// Check for new messages
			trMqttCheckMessages.update();

			// Send status data
			trMqttSendStatus.update();
		}
	#endif

	// If PIR sensor is not defined skip code in compiler
	#if PIR_ON
		if (bPirSensorEnabled) {
			getPirSensorState(digitalRead(PIR_PIN));
		}
	#endif

	// LED pattern
	// Display LED pattern if: Turn light ON enabled AND Nr. of LEDs per corner > 0 AND LED pattern is set AND LED pattern speed > 0
	if (checkDisplay() && beNrOfLedsCorner > 0 && beLedPattern != 0 && beLedPattern != 99 && beLedSpeed > 0) {
		gPatterns[gCurrentPatternNumber]();

		FastLED.show();
		FastLED.delay(1000 / beLedSpeed);
		EVERY_N_MILLISECONDS(20) { gHue++; } // slowly cycle the "base color" through the rainbow
	}
}
