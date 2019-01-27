/*
	Name:     Moodlite.ino
	Created:  27.12.2018
	Version:  2.0
	AuthorS:  Spigot (M.V.), Shiryou & Steve Wagg aka CdRsKuLL
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
#define DEBUG_ON						true		// Flag to turn On/Off debugging		[True = On, False = Off]
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
#define LED_PIN				6      // NodeMCU PIN s	
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
#define SSID_START						0x0
#define SSID_MAX						0x15
#define PASSWORD_START					(SSID_START + SSID_MAX)
#define PASSWORD_MAX					0x15
#define HOSTNAME_START					(PASSWORD_START + PASSWORD_MAX)
#define HOSTNAME_MAX					0x15
#define LED_BRIGHTNESS_START			(HOSTNAME_START + HOSTNAME_MAX)
#define LED_BRIGHTNESS_MAX				0x1
#define BACKLIGHT_START					(LED_BRIGHTNESS_START + LED_BRIGHTNESS_MAX)
#define BACKLIGHT_MAX					0x1
#define LED_COLOR_START					(BACKLIGHT_START + BACKLIGHT_MAX)
#define LED_COLOR_MAX					0x6
#define AUTDISPLAYON_START				(LED_COLOR_START + LED_COLOR_MAX)
#define AUTDISPLAYON_MAX				0x1
#define DISPLAYON_START					(AUTDISPLAYON_START + AUTDISPLAYON_MAX)
#define DISPLAYON_MAX					0x1
#define DISPLAYOFF_START				(DISPLAYON_START + DISPLAYON_MAX)
#define DISPLAYOFF_MAX					0x1
#define LED_SPEED_START					(DISPLAYOFF_START + DISPLAYOFF_MAX)
#define LED_SPEED_MAX					0x1
#define LED_PATTERN_START				(LED_SPEED_START + LED_SPEED_MAX)
#define LED_PATTERN_MAX					0x1
#define NUMBER_OF_LEDS_START			(LED_PATTERN_START + LED_PATTERN_MAX)
#define NUMBER_OF_LEDS_MAX				0x3
#define NTP_SERVER_START				(NUMBER_OF_LEDS_START + NUMBER_OF_LEDS_MAX)
#define NTP_SERVER_MAX					0x20
#define NUMBER_OF_LEDS_CORNER_START     (NTP_SERVER_START + NTP_SERVER_MAX)
#define NUMBER_OF_LEDS_CORNER_MAX		0x1
#define MQTT_HOSTNAME_START				(NUMBER_OF_LEDS_CORNER_START + NUMBER_OF_LEDS_CORNER_MAX)
#define MQTT_HOSTNAME_MAX				0x15
#define MQTT_PORT_START					(MQTT_HOSTNAME_START + MQTT_HOSTNAME_MAX)
#define MQTT_PORT_MAX					0x0A
#define MQTT_USERNAME_START				(MQTT_PORT_START + MQTT_PORT_MAX)
#define MQTT_USERNAME_MAX				0x15
#define MQTT_USERPASSWORD_START			(MQTT_USERNAME_START + MQTT_USERNAME_MAX)
#define MQTT_USERPASSWORD_MAX			0x15
#define MQTT_CLIENT_ID_START			(MQTT_USERPASSWORD_START + MQTT_USERPASSWORD_MAX)
#define MQTT_CLIENT_ID_MAX				0x15
#define PIR_SENSOR_ENABLED_START		(MQTT_CLIENT_ID_START + MQTT_CLIENT_ID_MAX)
#define PIR_SENSOR_ENABLED_MAX			0x1
#define LED_COLOR_PATTERN_START         (PIR_SENSOR_ENABLED_START + PIR_SENSOR_ENABLED_MAX)
#define LED_COLOR_PATTERN_MAX           0x1

// --- Constants ---
// Timers
// 1s
const int TR1S = 1000;
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
const char* cMqttSetTopic[11] = { "Moodlite/Brightness/Set", "Moodlite/Pattern/Set", "Moodlite/FixedPattern/Set", "Moodlite/PatternColor/Set", "Moodlite/Color/Set", "Moodlite/Speed/Set", "Moodlite/Backlight/Set", "Moodlite/DisplayOnEnabled/Set",
								"Moodlite/DisplayOn/Set", "Moodlite/DisplayOff/Set", "Moodlite/PirSensorEndabled/Set"
};
// Topics for MQTT client to GET values from Moodlite
const char* cMqttGetTopic[11] = { "Moodlite/Brightness/Get", "Moodlite/Pattern/Get", "Moodlite/FixedPattern/Get", "Moodlite/PatternColor/Get", "Moodlite/Color/Get", "Moodlite/Speed/Get", "Moodlite/Backlight/Get", "Moodlite/DisplayOnEnabled/Get",
								"Moodlite/DisplayOn/Get", "Moodlite/DisplayOff/Get", "Moodlite/PirSensorEndabled/Get"
};

// String sMqttValue = "";
boolean bMqttConnError = false;

// PIR sensor
// The time the sensor calibrates (10s - 60s)
int iPirCalibrationTime = 10;
// The time when the sensor outputs a low impulse
int iPirLowIn;
// The amount of milliseconds the sensor has to be low before we assume all motion has stopped
int iPirPause = 5000;

boolean bPirLockLow = true;
boolean bPirTakeLowTime;
boolean bPirBacklightOn = false;
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
SimplePatternList gPatterns = { &fullColor, &standard, &confetti, &sinelon, &juggle, &bpm, &kitt, &plasma, &blendwave, &inoise8_fire, &rainbow_beat, &rainbow, &rainbowWithGlitter };
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

// Timer
Timer trNtpCheckServer;
Timer trMqttCheckMessages;
Timer trMqttSendStatus;

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
				trMqttCheckMessages.every(TR1S, mqttCheckMessage, (void*)0);
				// Send MQTT statuses
				trMqttSendStatus.every(TR30M, mqttSendStatus, (void*)0);

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

			// Sensor time to calibrate
			DEBUG("Calibrating sensor ");

			for (int i = 0; i < iPirCalibrationTime; i++) {
				DEBUG(".");
				delay(1000);
			}

			DEBUGLN(" done.");
			DEBUGLN("PIR sensor active.");
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

// ----------------------------------------------------

// ------------------- Functions ----------------------
// Initialize WiFi
void initWiFi() {
	DEBUGLN("--- initWiFi - Start ---");
	// Disconnect() needs to be done before new connection
	WiFi.disconnect();

	MDNS.begin(cHostName);

	#if STATIC_IP
		if (strlen(cWifiSsid) > 0) {
			// Set base config
			// Check if DNS is configured
			if ((int)ipaWifiDns[0] > 0) {
				WiFi.config(ipaWifiIP, ipaWifiDns, ipaWifiGateway, ipaWifiSubnet);
			}
			else {
				WiFi.config(ipaWifiIP, ipaWifiGateway, ipaWifiSubnet);

				// If DNS is not configured don't use NTP
				if (NTP_ON) {
					DEBUGLN("NTP disabled due to DNS not configured.");
				}

				sNtpServer = "";
				// Disable auto LED On/Off
				bAutDisplayEnabled = false;
				beDisplayOn = 0;
				beDisplayOff = 0;
			}

			// Set Hostname
			#if ESP8266
				WiFi.hostname(cHostName);
			#elif ESP32
				WiFi.setHostname(cHostName);
			#endif

			bool bWifiInRange = false;
			int iNumSsid = WiFi.scanNetworks(false);

			if (iNumSsid == 0) {
				DEBUGLN("No wifi networks found.");
			}
			else {
				for (int i = 0; i < iNumSsid; ++i) {
					if (WiFi.SSID(i) == cWifiSsid) {
						bWifiInRange = true;
					}
				}
			}

			if (bWifiInRange) {
				if (strlen(cWifiPassword) > 0) {
					WiFi.begin(cWifiSsid, cWifiPassword);
				}
				else {
					WiFi.begin(cWifiSsid);
				}

				DEBUGLN("Connecting to Wifi. Please wait.");
				delay(10000);
			}
			else {
				DEBUGLN("Wifi SSID not found.");
			}

		}
	#else
		// Wifi AP Config
		WiFi.mode(WIFI_AP_STA);

		WiFi.softAP(cHostName, WIFI_AP_PASSWORD);
		WiFi.softAPConfig(ipaWifiApIP, ipaWifiApGateway, ipaWifiApSubnet);

		if (strlen(cWifiSsid) > 0) {
			// Set Hostname
			#if ESP8266
				WiFi.hostname(cHostName);
			#elif ESP32
				WiFi.setHostname(cHostName);
			#endif

			if (strlen(cWifiPassword) > 0) {
				WiFi.begin(cWifiSsid, cWifiPassword);
			}
			else {
				WiFi.begin(cWifiSsid);
			}

			DEBUGLN("Connecting to Wifi. Please wait.");
			delay(10000);
		}
	#endif

	// Status:
	// WL_NO_SHIELD = 255,
	// WL_IDLE_STATUS = 0,
	// WL_NO_SSID_AVAIL = 1
	// WL_SCAN_COMPLETED = 2
	// WL_CONNECTED = 3
	// WL_CONNECT_FAILED = 4
	// WL_CONNECTION_LOST = 5
	// WL_DISCONNECTED = 6
	DEBUG("Wifi status: ");
	DEBUGLN(WiFi.status());

	if (WiFi.status() == WL_CONNECTED)
		DEBUGLN("Wifi network started succesfully.");
	else if (WiFi.status() == WL_DISCONNECTED)
		DEBUGLN("Error conecting to wifi network.");

	DEBUGLN("--- initWiFi - End ---");
}

// EEPROM functions
// Read byte from EEPROM
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
String readStringFromEEPROM(int iStart, int iMax) {
	int iEnd = iStart + iMax;
	String sValue = "";

	for (int i = iStart; i < iEnd; i++) {
		byte readByte = EEPROM.read(i);
		if (readByte > 0 && readByte < 128) {
			sValue += char(readByte);
		}
		else {
			break;
		}
	}
	return (sValue);
}

// Write string to EEPROM
void writeStringToEEPROM(String sValue, int iStart, int iMax) {
	int iEnd = iStart + iMax;

	for (int i = iStart; i < iEnd; i++) {
		if (i - iStart < sValue.length()) {
			EEPROM.write(i, sValue[i - iStart]);
		}
		else {
			EEPROM.write(i, 0);
			break;
		}
	}

	EEPROM.commit();
}


// Returns true if the address is between the
// minimum and maximum allowed values, false otherwise.
boolean checkEEPROMAdr(int iAddress) {
	return ((iAddress >= EEPROM_MIN_ADDR) && (iAddress <= EEPROM_MAX_ADDR));
}

// Writes a sequence of bytes to eeprom starting at the specified address.
// Returns true if the whole array is successfully written.
// Returns false if the start or end addresses aren't between
// the minimum and maximum allowed values.
// When returning false, nothing gets written to eeprom.
boolean writeBytesToEEPROM(const byte* beArray, int iStartAddress, int iNumerOfBytes) {
	// counter
	int i;

	// both first byte and last byte addresses must fall within
	// the allowed range 
	if (!checkEEPROMAdr(iStartAddress) || !checkEEPROMAdr(iStartAddress + iNumerOfBytes)) {
		return false;
	}

	for (i = 0; i < iNumerOfBytes; i++) {
		EEPROM.write(iStartAddress + i, beArray[i]);
	}

	EEPROM.commit();

	return true;
}

// Writes a string starting at the specified address.
// Returns true if the whole string is successfully written.
// Returns false if the address of one or more bytes fall outside the allowed range.
// If false is returned, nothing gets written to the eeprom.
boolean writeCharToEEPROM(const char* cString, int iAddress) {
	int numBytes; // actual number of bytes to be written

	//write the string contents plus the string terminator byte (0x00)
	numBytes = strlen(cString) + 1;
	return writeBytesToEEPROM((const byte*)cString, iAddress, numBytes);
}

// Reads a string starting from the specified address.
// Returns true if at least one byte (even only the string terminator one) is read.
// Returns false if the start address falls outside the allowed range or declare buffer size is zero.
// 
// The reading might stop for several reasons:
// - no more space in the provided buffer
// - last eeprom address reached
// - string terminator byte (0x00) encountered.
boolean readCharFromEEPROM(char* cBuffer, int iAddress, int iBufferSize) {
	byte ch; // byte read from eeprom
	int bytesRead; // number of bytes read so far

	if (!checkEEPROMAdr(iAddress)) { // check start address
		return false;
	}

	if (cBuffer == 0) { // how can we store bytes in an empty buffer ?
		return false;
	}

	// is there is room for the string terminator only, no reason to go further
	if (iBufferSize == 1) {
		cBuffer[0] = 0;
		return true;
	}

	bytesRead = 0; // initialize byte counter
	ch = EEPROM.read(iAddress + bytesRead); // read next byte from eeprom
	cBuffer[bytesRead] = ch; // store it into the user buffer	
	bytesRead++; // increment byte counter

	// stop conditions:
	// - the character just read is the string terminator one (0x00)
	// - we have filled the user buffer
	// - we have reached the last eeprom address
	while ((ch != 0x00) && (bytesRead < iBufferSize) && ((iAddress + bytesRead) <= EEPROM_MAX_ADDR)) {
		// if no stop condition is met, read the next byte from eeprom
		ch = EEPROM.read(iAddress + bytesRead);
		cBuffer[bytesRead] = ch; // store it into the user buffer
		bytesRead++; // increment byte counter
	}

	// make sure the user buffer has a string terminator, (0x00) as its last byte
	if ((ch != 0x00) && (bytesRead >= 1)) {
		cBuffer[bytesRead - 1] = 0;
	}
	return true;
}

// Erase EEPROM
void eraseEEPROM() {
	DEBUGLN("--- eraseEEPROM - Start ---");

	for (int i = 0; i < EEPROM.length(); i++) {
		EEPROM.write(i, 0);
	}
	EEPROM.commit();

	DEBUGLN("--- eraseEEPROM - End ---");

	ESP.restart();
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

	// If static IP is deffined skip WiFi settings load from memory
	#if  STATIC_IP
	#else	
		// Hostname
		char cConfiguredHostname[21] = "";
		readCharFromEEPROM(cConfiguredHostname, HOSTNAME_START, HOSTNAME_MAX);

		if (strlen(cConfiguredHostname) > 0) {
			sprintf(cHostName, "%s", cConfiguredHostname);

			DEBUGLN("Hostname loaded from memory.");
		}
		else {
			DEBUGLN("No Hostname stored in memory, loading default Hostname.");
		}

		DEBUG("Wifi Hostname: ");
		DEBUGLN(cHostName);

		// WiFi SSID
		readCharFromEEPROM(cWifiSsid, SSID_START, SSID_MAX);

		DEBUG("Wifi ssid: ");
		DEBUGLN(cWifiSsid);

		// WiFi Password
		readCharFromEEPROM(cWifiPassword, PASSWORD_START, PASSWORD_MAX);

		DEBUG("Wifi password: ");
		DEBUGLN(cWifiPassword);
	#endif

	if (strlen(cWifiSsid) > 0) {
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

		// Manual Display On
		bBacklight = readByteFromEEPROM(BACKLIGHT_START);

		// Automatic Display On
		bAutDisplayEnabled = readByteFromEEPROM(AUTDISPLAYON_START);
		beDisplayOn = readByteFromEEPROM(DISPLAYON_START);
		beDisplayOff = readByteFromEEPROM(DISPLAYOFF_START);

		// PIR senor (motion sensor) enabled
		bPirSensorEnabled = readByteFromEEPROM(PIR_SENSOR_ENABLED_START);

		// LED Brightness
		beLedBrightness = beLedOldBrightness = readByteFromEEPROM(LED_BRIGHTNESS_START);

		// LED Speed
		beLedSpeed = readByteFromEEPROM(LED_SPEED_START);

		//LED Pattern
		beLedPattern = readByteFromEEPROM(LED_PATTERN_START);

		// NTP Server
		sNtpServer = readStringFromEEPROM(NTP_SERVER_START, NTP_SERVER_MAX);

		// MQTT Hostname
		readCharFromEEPROM(cMqttHostName, MQTT_HOSTNAME_START, MQTT_HOSTNAME_MAX);

		// MQTT port
		readCharFromEEPROM(cMqttPort, MQTT_PORT_START, MQTT_PORT_MAX);
		iMqttPort = atoi(cMqttPort);

		// MQTT User name
		readCharFromEEPROM(cMqttUserName, MQTT_USERNAME_START, MQTT_USERNAME_MAX);
		 
		// MQTT User password
		readCharFromEEPROM(cMqttUserPassword, MQTT_USERPASSWORD_START, MQTT_USERPASSWORD_MAX);

		// MQTT Client ID
		readCharFromEEPROM(cMqttClientId, MQTT_CLIENT_ID_START, MQTT_CLIENT_ID_MAX);

        //LED Color Pattern
        beLedColorPattern = readByteFromEEPROM(LED_COLOR_PATTERN_START);

		DEBUG("Manual Display On: ");
		DEBUGLN(bBacklight);
		DEBUG("Automatic Display On: ");
		DEBUGLN(bAutDisplayEnabled);
		DEBUG("Display On: ");
		DEBUGLN(beDisplayOn);
		DEBUG("Display Off: ");
		DEBUGLN(beDisplayOff);
		DEBUG("Pir sensor enabled: ");
		DEBUGLN(bPirSensorEnabled);
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
        DEBUG("Pattern color: ");
        DEBUGLN(beLedColorPattern);
		DEBUG("LED color: ");
		DEBUGLN(sLedHexColor);
		DEBUG("NTP Server: ");
		DEBUGLN(sNtpServer);
		DEBUG("MQTT Hostname: ");
		DEBUGLN(cMqttHostName);
		DEBUG("MQTT port: ");
		DEBUGLN(cMqttPort);
		DEBUG("MQTT User name: ");
		DEBUGLN(cMqttUserName);
		DEBUG("MQTT User password: ");
		DEBUGLN(cMqttUserPassword);
		DEBUG("MQTT Client ID: ");
		DEBUGLN(cMqttClientId);
	}

	DEBUGLN("--- initEEPROMData - End ---");

	// If static IP is deffined and if wrong parameters for LEDs were stored, reload default values (e.g. first start)
	// The minimum is 1 LED per corner and total 3 LEDs
	#if  STATIC_IP
		if ((beNrOfLedsCorner < 1) || (iNrOfLeds < 3)) {
			setDefaulValues();
			ESP.restart();
		}
	#endif
}

// Set default values to EEPROM
void setDefaulValues() {
	DEBUGLN("--- setDefaulValues - Start ---");

	writeByteToEEPROM(1, BACKLIGHT_START);
	writeByteToEEPROM(20, LED_BRIGHTNESS_START);
	writeCharToEEPROM("FFFFFF", LED_COLOR_START);
	writeByteToEEPROM(1, NUMBER_OF_LEDS_CORNER_START);
	writeCharToEEPROM("3", NUMBER_OF_LEDS_START);
	writeByteToEEPROM(50, LED_SPEED_START);

	DEBUGLN("--- setDefaulValues - End ---");
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

	json += cWifiSsid;
	json += "\",\"password\":\"";
	json += cWifiPassword;
	json += "\",\"hostname\":\"";
	json += cHostName;
	json += "\"}}";

	client->text(json);

	DEBUGLN("--- sendWifiValues - End ---");
}

void sendSettingsValues(AsyncWebSocketClient *client) {
	DEBUGLN("--- sendSettingsValues - Start ---");

	String sNtpDateTime = String(hour()) + " : " + String(minute()) + " : " + String(second());

	String json = String("{\"type\":\"sv.init.settings\",\"value\":{") +
		"\"ntp_server\":\"" + sNtpServer + "\"," +
		"\"ntp_date_time\":\"" + sNtpDateTime + "\"," +
		"\"led_strip_nr_of_leds_corner\":" + beNrOfLedsCorner + "," +
		"\"led_strip_nr_of_leds\":" + iNrOfLeds + "," +
		"\"mqtt_host_name\":\"" + cMqttHostName + "\"," +
		"\"mqtt_port\":\"" + cMqttPort + "\"," +
		"\"mqtt_user_name\":\"" + cMqttUserName + "\"," +
		"\"mqtt_user_password\":\"" + cMqttUserPassword + "\"," +
		"\"mqtt_client_id\":\"" + cMqttClientId + "\"" +
		"}}";

	client->text(json);

	DEBUGLN("--- sendSettingsValues - End ---");
}

void sendLedValues(AsyncWebSocketClient *client) {
	DEBUGLN("--- sendLedValues - Start ---");

	String json = String("{\"type\":\"sv.init.leds\",\"value\":{") +
		"\"pattern_color\":" + beLedColorPattern + "," +
		"\"patterns\":" + beLedPattern + "," +
		"\"fixed_patterns\":" + beLedPattern + "," +
		"\"aut_display_enabled\":" + bAutDisplayEnabled + "," +
		"\"display_on\":" + beDisplayOn + "," +
		"\"display_off\":" + beDisplayOff + "," +
		"\"backlight\":" + bBacklight + "," +
		"\"pir_sensor_endabled\":" + bPirSensorEnabled + "," +
		"\"colorpickerfield_led_strip\":\"" + sLedHexColor + "\"," +
		"\"led_strip_brightness\":" + beLedBrightness + "," +
		"\"led_strip_speed\":" + beLedSpeed +
		"}}";

	client->text(json);
	
	DEBUGLN("--- sendLedValues - End ---");
}

void updateValue(String pair) {
	DEBUGLN("--- updateValue - Start ---");

	int index = pair.indexOf(':');

	// _key has to hang around because key points to an internal data structure
	String      _key = pair.substring(0, index);
	const char *key = _key.c_str();
	String      value = pair.substring(index + 1);

	char cMqttValue[21];

	// Automatic Display On
	// True = On
	// False = Off
	if (strcmp("aut_display_enabled", key) == 0) {
		DEBUG("aut_display_enabled: ");
		DEBUGLN(value);

		// Convert string value "true"/"false" to boolean for further calculation
		const char *to_find = value.c_str(); // string we want to find

		if (strcmp(to_find, "true") == 0) { // this is the key: a match returns 0
			bAutDisplayEnabled = true;
		}
		else {
			bAutDisplayEnabled = false;
		}

		// Publish new value
		sprintf(cMqttValue, "%i", bAutDisplayEnabled);
		mqttClient.publish(cMqttGetTopic[7], cMqttValue, true);
	}

	// Display On Hour
	// Hour when backlight is on
	else if (strcmp("display_on", key) == 0) {
		DEBUG("display_on: ");
		DEBUGLN(value);

		beDisplayOn = value.toInt();

		// Publish new value
		sprintf(cMqttValue, "%i", beDisplayOn);
		mqttClient.publish(cMqttGetTopic[8], cMqttValue, true);
	}

	// Hour when backlight is off
	// Display Off Hour
	else if (strcmp("display_off", key) == 0) {
		DEBUG("display_off: ");
		DEBUGLN(value);

		beDisplayOff = value.toInt();

		// Publish new value
		sprintf(cMqttValue, "%i", beDisplayOff);
		mqttClient.publish(cMqttGetTopic[9], cMqttValue, true);
	}

	// PIR sensor (motion sensor) On/Off
	// True = On
	// False = Off
	if (strcmp("pir_sensor_endabled", key) == 0) {
		DEBUG("pir_sensor_endabled: ");
		DEBUGLN(value);

		// Convert string value "true"/"false" to boolean for further calculation
		const char *to_find = value.c_str(); // string we want to find

		if (strcmp(to_find, "true") == 0) { // this is the key: a match returns 0
			bPirSensorEnabled = true;
		}
		else {
			bPirSensorEnabled = false;
		}

		// Publish new value
		sprintf(cMqttValue, "%i", bPirSensorEnabled);
		mqttClient.publish(cMqttGetTopic[10], cMqttValue, true);
	}

	// Manual Display On
	// True = On
	// False = Off
	else if (strcmp("backlight", key) == 0) {
		DEBUG("Manual Display On: ");
		DEBUGLN(value);

		// Convert string value "true"/"false" to boolean for further calculation
		const char *to_find = value.c_str(); // string we want to find

		if (strcmp(to_find, "true") == 0) { // this is the key: a match returns 0
			bBacklight = true;
		}
		else {
			bBacklight = false;
		}

		// If automatic Display On is turned Off
		if (!bAutDisplayEnabled) {
			// Brighntess change start
			bBrighntessChange = true;

			// if Manual Display On is On set brightness and colour to last value
			if (bBacklight) {
				beLedBrightness = beLedOldBrightness;
			}
			// if Manual Display On is Off save previous value and set brightness to 0
			else {
				beLedOldBrightness = beLedBrightness;
				beLedBrightness = 0;
			}
			// Change Rgb Led brightness
			FastLED.setBrightness(beLedBrightness);
			FastLED.show();

			// Restore pattern
			switchLedPattern(beLedPattern);

			// Brighntess change end
			bBrighntessChange = false;
		}

		// Publish new value
		sprintf(cMqttValue, "%i", bBacklight);
		mqttClient.publish(cMqttGetTopic[6], cMqttValue, true);
	}

	// Clock's color
	// Color HEX value
	else if (strcmp("colorpickerfield_led_strip", key) == 0) {
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

		// Publish new value
		sprintf(cMqttValue, "%i,%i,%i", stLedColors.beRed, stLedColors.beGreen, stLedColors.beBlue);
		mqttClient.publish(cMqttGetTopic[4], cMqttValue, true);
	}

	// Brightness
	// Led strip brightness value
	else if (strcmp("led_strip_brightness", key) == 0) {
		// Brighntess change start
		bBrighntessChange = true;

		DEBUG("led_strip_brightness: ");
		DEBUGLN(value);

		// If brightness value changed remember previous value
		if (abs(beLedBrightness - beLedOldBrightness) > 0)
			beLedOldBrightness = beLedBrightness;

		beLedBrightness = value.toInt();

		// Change Rgb LED brightness
		FastLED.setBrightness(beLedBrightness);
		FastLED.show();
		DEBUGLN(beLedBrightness);

		// Publish new value
		sprintf(cMqttValue, "%i", beLedBrightness);
		mqttClient.publish(cMqttGetTopic[0], cMqttValue, true);

		// Brighntess change end
		bBrighntessChange = false;
	}

	// Speed
	// LEd strip speed value
	else if (strcmp("led_strip_speed", key) == 0) {
		DEBUG("led_strip_speed: ");
		DEBUGLN(value);

		beLedOldSpeed = beLedSpeed;

		beLedSpeed = value.toInt();

		DEBUGLN(beLedSpeed);

		// Publish new value
		sprintf(cMqttValue, "%i", beLedSpeed);
		mqttClient.publish(cMqttGetTopic[5], cMqttValue, true);
	}

	// LED pattern color
	else if (strcmp("patterncolor", key) == 0) {
		DEBUG("patterncolor: ");
		DEBUGLN(value);

		beLedColorPattern = value.toInt();

		FastLED.clear();

		changeLedColorPatternParameters(beLedColorPattern);

		DEBUGLN(beLedColorPattern);

		// Publish new value
		sprintf(cMqttValue, "%i", beLedColorPattern);
		mqttClient.publish(cMqttGetTopic[3], cMqttValue, true);
	}

	// LED pattern
	else if (strcmp("pattern", key) == 0) {
		DEBUG("pattern: ");
		DEBUGLN(value);

		beLedPattern = value.toInt();

		FastLED.clear();

		changeLedPatternParameters(beLedPattern);

		DEBUGLN(beLedPattern);

		// MQTT
		// Clear Fixced pattern value
		sprintf(cMqttValue, "%i", 0);
		mqttClient.publish(cMqttGetTopic[2], cMqttValue, true);
		// Publish new value
		sprintf(cMqttValue, "%i", beLedPattern);
		mqttClient.publish(cMqttGetTopic[1], cMqttValue, true);
	}

	// LED fixed pattern
	else if (strcmp("fixedpattern", key) == 0) {
		DEBUG("fixedpattern: ");
		DEBUGLN(value);

		beLedPattern = value.toInt();

		FastLED.clear();

		changeLedFixedPatternParameters(beLedPattern);

		DEBUGLN(beLedPattern);

		// MQTT
		// Clear Pattern value
		sprintf(cMqttValue, "%i", 0);
		mqttClient.publish(cMqttGetTopic[1], cMqttValue, true);
		// Publish new values
		sprintf(cMqttValue, "%i", beLedPattern);
		mqttClient.publish(cMqttGetTopic[2], cMqttValue, true);
	}

	// Store as default values
	else if (strcmp("saveDefaultValues", key) == 0) {
		DEBUG("saveDefaultValues: ");
		DEBUGLN(value);

		if (value.toInt() == 1) {
			DEBUGLN("Store data");
			DEBUG("Automatic Display On: ");
			DEBUGLN(bAutDisplayEnabled);
			writeByteToEEPROM((int)bAutDisplayEnabled, AUTDISPLAYON_START);
			DEBUG("Display On hour: ");
			DEBUGLN(beDisplayOn);
			writeByteToEEPROM(beDisplayOn, DISPLAYON_START);
			DEBUG("Display Off hour: ");
			DEBUGLN(beDisplayOff);
			writeByteToEEPROM(beDisplayOff, DISPLAYOFF_START);
			DEBUG("Manual Display On: ");
			DEBUGLN(bBacklight);
			writeByteToEEPROM((int)bBacklight, BACKLIGHT_START);
			DEBUG("PIR (motion) sensor enabled: ");
			DEBUGLN(bPirSensorEnabled);
			writeByteToEEPROM((int)bPirSensorEnabled, PIR_SENSOR_ENABLED_START);
			DEBUG("Brightness: ");
			DEBUGLN(beLedBrightness);
			writeByteToEEPROM(beLedBrightness, LED_BRIGHTNESS_START);
			DEBUG("LED pattern speed: ");
			DEBUGLN(beLedSpeed);
			writeByteToEEPROM(beLedSpeed, LED_SPEED_START);
			DEBUG("LED pattern: ");
			DEBUGLN(beLedPattern);
			writeByteToEEPROM(beLedPattern, LED_PATTERN_START);
			DEBUG("LED color pattern: ");
			DEBUGLN(beLedColorPattern);
			writeByteToEEPROM(beLedColorPattern, LED_COLOR_PATTERN_START);
			DEBUG("Pattern 0 color: ");
			DEBUGLN(sLedHexColor);
			writeStringToEEPROM(sLedHexColor, LED_COLOR_START, LED_COLOR_MAX);
		}
	}

	// Set NTP server
	else if (strcmp("setNtpServer", key) == 0) {
		DEBUG("setNtpServer: ");
		DEBUGLN(value);

		sNtpServer = value;

		DEBUGLN(sNtpServer);
	}

	// Set number of LEDs per corner
	else if (strcmp("setNrOfLedsCorner", key) == 0) {
		DEBUG("setNrOfLedsCorner: ");
		DEBUGLN(value);

		sNrOfLedsCorner = value;
		beNrOfLedsCorner = value.toInt();

		DEBUGLN(beNrOfLedsCorner);
	}

	// Set total number of LEDs
	else if (strcmp("setNrOfLeds", key) == 0) {
		DEBUG("setNrOfLeds: ");
		DEBUGLN(value);

		sNrOfLeds = value;
		iNrOfLeds = value.toInt();

		DEBUGLN(iNrOfLeds);
	}

	// Save LEDs settings
	else if (strcmp("saveLedsSettings", key) == 0) {
		DEBUG("saveLedsSettings: ");
		DEBUGLN(value);

		if (value.toInt() == 1) {
			writeStringToEEPROM(sNtpServer, NTP_SERVER_START, NTP_SERVER_MAX);
			writeByteToEEPROM(beNrOfLedsCorner, NUMBER_OF_LEDS_CORNER_START);
			writeStringToEEPROM(sNrOfLeds, NUMBER_OF_LEDS_START, NUMBER_OF_LEDS_MAX);
		}
	}

	// MQTT
	// MQTT Hostname
	else if (strcmp("setMqttHostName", key) == 0) {
		DEBUG("setMqttHostName: ");
		DEBUGLN(value);	

		value.toCharArray(cMqttHostName, 21);

		DEBUGLN(cMqttHostName);
	}

	// MQTT Port
	else if (strcmp("setMqttPort", key) == 0) {
		DEBUG("setMqttPort: ");
		DEBUGLN(value);

		value.toCharArray(cMqttPort, 11);

		DEBUGLN(cMqttPort);
	}

	// MQTT User name
	else if (strcmp("setMqttUserName", key) == 0) {
		DEBUG("setMqttUserName: ");
		DEBUGLN(value);

		value.toCharArray(cMqttUserName, 21);

		DEBUGLN(cMqttUserName);
	}

	// MQTT User password
	else if (strcmp("setMqttUserPassword", key) == 0) {
		DEBUG("setMqttUserPassword: ");
		DEBUGLN(value);

		value.toCharArray(cMqttUserPassword, 21);

		DEBUGLN(cMqttUserPassword);
	}

	// MQTT Client ID
	else if (strcmp("setMqttClient", key) == 0) {
		DEBUG("setMqttClient: ");
		DEBUGLN(value);

		value.toCharArray(cMqttClientId, 21);

		DEBUGLN(cMqttClientId);
	}

	// Save MQTT settings
	else if (strcmp("saveMqttSettings", key) == 0) {
		DEBUG("saveMqttSettings: ");
		DEBUGLN(value);

		if (value.toInt() == 1) {
			writeCharToEEPROM(cMqttHostName, MQTT_HOSTNAME_START);
			writeCharToEEPROM(cMqttPort, MQTT_PORT_START);
			writeCharToEEPROM(cMqttUserName, MQTT_USERNAME_START);
			writeCharToEEPROM(cMqttUserPassword, MQTT_USERPASSWORD_START);
			writeCharToEEPROM(cMqttClientId, MQTT_CLIENT_ID_START);
		}
	}

	// Restart
	else if (strcmp("restart", key) == 0) {
		DEBUG("restart: ");
		DEBUGLN(value);

		if (value.toInt() == 1) {
			beLedPattern = 99;
			DEBUGLN(99);
			changeLedPatternParameters(beLedPattern);
		}
	}

	DEBUGLN("--- updateValue - End ---");
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

	#if ESP8266
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
	#elif ESP32
		// ESP32 Info table
		response += getTableHead2Col("ESP32 information", "Name", "Value");

		//TODO: ESP32 Info table Need to be coded. I cant find anithing about this for ESP32 but is working anyways so...

		response += getTableFoot();
	#endif

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

	AsyncWebParameter *p = request->getParam("ssid", true);
	if (p) {
		String _ssid = p->value();

		writeStringToEEPROM(_ssid, SSID_START, SSID_MAX);

		DEBUG("SSID: ");
		DEBUGLN(_ssid.c_str());
	}
	else {
		DEBUG("Erasing SSID.");

		writeStringToEEPROM("", SSID_START, SSID_MAX);
	}

	p = request->getParam("password", true);
	if (p) {
		String _password = p->value();

		writeStringToEEPROM(_password, PASSWORD_START, PASSWORD_MAX);

		DEBUG("Password: ");
		DEBUGLN(_password.c_str());
	}
	else {
		DEBUG("Erasing password.");

		writeStringToEEPROM("", PASSWORD_START, PASSWORD_MAX);
	}

	p = request->getParam("hostname", true);
	if (p) {
		String _hostname = p->value();

		writeStringToEEPROM(_hostname, HOSTNAME_START, HOSTNAME_MAX);

		DEBUG("Hostname: ");
		DEBUGLN(_hostname.c_str());
	}
	else {
		DEBUG("Erasing Hostname.");

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

// Convert RGB color to HEX
String rgbToHex(byte beRed, byte beGreen, byte beBlue) {
	char hexColor[16] = "";

	snprintf(hexColor, sizeof hexColor, "%02x%02x%02x", beRed, beGreen, beBlue);

	//long hexColor = ((long)beRed << 16L) | ((long)beGreen << 8L) | (long)beBlue;
	//sprintf(hexColor, "#%02x%02x%02x", beRed, beGreen, beBlue);

	return String(hexColor);
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

		case 99:
			FastLED.clear();
			turnOffLeds(0, iNrOfLeds);
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
	boolean bRet = true;

	// If Brighntness change is not executing
	if (!bBrighntessChange) {
		// Turn LED On/Off
		// On = Automatic Display On is ON and Actual hour is within Display On hours limits
		//      OR
		//      Automatic Display On is ON and Manual Display On is on
		//		OR
		//		PIR sensor activated
		// Off = everything else 
		if ((bAutDisplayEnabled && checkHour()) || (!bAutDisplayEnabled && bBacklight) || (bPirBacklightOn)) {
			// On change
			// beLedBrightness == STANDBY_LED_BRIGHTNESS -> Standby mode
			if (beLedBrightness == STANDBY_LED_BRIGHTNESS) {
				beLedBrightness = beLedOldBrightness;
			}
		}
		else {
			// On change
			if (beLedBrightness > STANDBY_LED_BRIGHTNESS) {
				beLedOldBrightness = beLedBrightness;
			}

			stadbyMode();

			bRet = false;
		}

		// Execute only if device is configured
		if (strlen(cWifiSsid) > 0) {
			// Change Rgb Led brightness
			FastLED.setBrightness(beLedBrightness);
			FastLED.show();
		}
	}

	return(bRet);
}

// NTP Server
void ntpCheckServer(void *context) {
	DEBUGLN("--- ntpCheckServer - Start ---");

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
		DEBUGLN("Calling ntpCheckServer");
		ntpCheckServer((void *)0);
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

	DEBUGLN("--- ntpCheckServer - End ---");
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
void fillLedsFromPaletteColors(uint8_t ui8ColorIndex)
{
    uint8_t ui8Brightness = 255;

    for (int i = 0; i < ((iNrOfLeds / beNrOfLedsCorner) + beNrOfLedsCorner); i++) {
        for (int j = i * beNrOfLedsCorner; j < ((i * beNrOfLedsCorner) + beNrOfLedsCorner); j++) {
            crgbLeds[j] = ColorFromPalette(crgbCurrentPalette, ui8ColorIndex, ui8Brightness, currentBlending);
        }
        ui8ColorIndex += 3;
    }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
void setPattern(int numPattern){
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (numPattern) % ARRAY_SIZE( gPatterns);
}

void standard(){
    fillLedsFromPaletteColors(gHue);
}

void rainbow(){
    // FastLED's built-in rainbow generator
    fill_rainbow( crgbLeds, iNrOfLeds, gHue, 7);
}

void rainbowWithGlitter(){
    // built-in FastLED rainbow, plus some random sparkly glitter
    rainbow();
    addGlitter(80);
}
void addGlitter( fract8 chanceOfGlitter){
    if( random8() < chanceOfGlitter) {
        crgbLeds[ random16(iNrOfLeds) ] += CRGB::White;
    }
}

void fullColor(){
    setLedsColor(0, iNrOfLeds, crgbLedColors);
    FastLED.show();
}

void confetti(){
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy( crgbLeds, iNrOfLeds, 10);
    int pos = random16(iNrOfLeds);
    crgbLeds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon(){
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( crgbLeds, iNrOfLeds, 20);
    int pos = beatsin16( 13, 0, iNrOfLeds-1 );
    crgbLeds[pos] += CHSV( gHue, 255, 192);
}

void juggle(){
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy( crgbLeds, iNrOfLeds, 20);
    byte dothue = 0;
    for( int i = 0; i < 8; i++) {
        crgbLeds[beatsin16( i+7, 0, iNrOfLeds-1 )] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}

void bpm(){
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < iNrOfLeds; i++) { //9948
        crgbLeds[i] = ColorFromPalette(crgbCurrentPalette, gHue+(i*2), beat-gHue+(i*10));
    }
}

void kitt(){
    // a colored dot sweeping back and forth, with fading trails
    uint8_t BeatsPerMinute = 62;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    fadeToBlackBy( crgbLeds, iNrOfLeds, 20);
    int pos = beatsin16( 13, 0, iNrOfLeds-1 );
    crgbLeds[pos] += ColorFromPalette(crgbCurrentPalette, gHue+(pos*2), beat-gHue+(pos*10));
}

#define qsuba(x, b)  ((x>b)?x-b:0)																	// Analog Unsigned subtraction macro. if result <0, then => 0
void plasma(){																						// This is the heart of this program. Sure is short. . . and fast.
    int thisPhase = beatsin8(6,-64,64);																// Setting phase change for a couple of waves.
    int thatPhase = beatsin8(7,-64,64);
    
    for (int k=0; k<iNrOfLeds; k++) {																// For each of the LED's in the strand, set a brightness based on a wave as follows:
    
        int colorIndex = cubicwave8((k*23)+thisPhase)/2 + cos8((k*15)+thatPhase)/2;					// Create a wave and add a phase change and add another wave with its own phase change.. Hey, you can even change the frequencies if you wish.
        int thisBright = qsuba(colorIndex, beatsin8(7,0,96));										// qsub gives it a bit of 'black' dead space by setting sets a minimum value. If colorIndex < current value of beatsin8(), then bright = 0. Otherwise, bright = colorIndex..
        
        crgbLeds[k] = ColorFromPalette(crgbCurrentPalette, colorIndex, thisBright, LINEARBLEND);	// Let's now add the foreground colour.
    }
}

CRGB clr1;
CRGB clr2;
uint8_t speed;
uint8_t loc1;
void blendwave(){
    speed = beatsin8(6,0,255);
    
    clr1 = blend(CHSV(beatsin8(3,0,255),255,255), CHSV(beatsin8(4,0,255),255,255), speed);
    clr2 = blend(CHSV(beatsin8(4,0,255),255,255), CHSV(beatsin8(3,0,255),255,255), speed);
    
    loc1 = beatsin8(10,0,iNrOfLeds-1);
    
    fill_gradient_RGB(crgbLeds, 0, clr2, loc1, clr1);
    fill_gradient_RGB(crgbLeds, loc1, clr2, iNrOfLeds-1, clr1);
}

uint32_t xscale = 20;																									// How far apart they are
uint32_t yscale = 3;																									// How fast they move
uint8_t indexFire = 0;
void inoise8_fire() {
    for(int i = 0; i < NUM_LEDS; i++) {
        indexFire = inoise8(i*xscale,millis()*yscale*iNrOfLeds/255);													// X location is constant, but we move along the Y at the rate of millis()
        crgbLeds[i] = ColorFromPalette(crgbCurrentPalette, min(i*(indexFire)>>6, 255), i*255/iNrOfLeds, LINEARBLEND);	// With that value, look up the 8 bit colour palette value and assign it to the current LED.
    }																													// The higher the value of i => the higher up the palette index (see palette definition).
																														// Also, the higher the value of i => the brighter the LED.
}

void setFirePattern() {
	crgbCurrentPalette[0] = CRGB::Black;
	crgbCurrentPalette[1] = CRGB::Black;
	crgbCurrentPalette[2] = CRGB::Black;
	crgbCurrentPalette[3] = CHSV(0, 255, 4);
	crgbCurrentPalette[4] = CHSV(0, 255, 8);
	crgbCurrentPalette[14] = CRGB::Gray;
	crgbCurrentPalette[15] = CRGB::Gray;
}

void rainbow_beat() {
    uint8_t beatA = beatsin8(17, 0, 255);						// Starting hue
    uint8_t beatB = beatsin8(13, 0, 255);
    fill_rainbow(crgbLeds, iNrOfLeds, (beatA+beatB)/2, 8);		// Use FastLED's fill_rainbow routine.
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


// MQTT functions
// MQTT Callback
void mqttCallback(char* topic, byte* payload, unsigned int length) {
	DEBUGLN("--- mqttCallback - Start ---");

	char cMqttValue[21];

	DEBUG("MQTT Message arrived [");
	DEBUG(topic);
	DEBUGLN("] ");

	for (int i = 0; i < length; i++) {
		cMqttValue[i] = (char)payload[i];
		cMqttValue[i + 1] = '\0';
	}

	// Brightness
	if (strcmp(topic, cMqttSetTopic[0]) == 0) {
		DEBUG("MQTT Brightness: ");
		DEBUGLN(cMqttValue);

		// If brightness value changed remember previous value
		if (abs(beLedBrightness - atoi(cMqttValue)) > 0) {
			beLedOldBrightness = beLedBrightness;
		}

		beLedBrightness = atoi(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[0], cMqttValue, true);

		FastLED.setBrightness(beLedBrightness);
		FastLED.show();
	}

	// Pattern
	else if (strcmp(topic, cMqttSetTopic[1]) == 0) {
		DEBUG("MQTT Pattern: ");
		DEBUGLN(cMqttValue);		

		// Clear Pattern value
		if (atoi(cMqttValue) > 0) {
			mqttClient.publish(cMqttGetTopic[2], "0", true);
		}

		if (atoi(cMqttValue) > 0) {
			beLedPattern = atoi(cMqttValue);

			FastLED.clear();

			changeLedPatternParameters(beLedPattern);
		}

		// Publish new value
		mqttClient.publish(cMqttGetTopic[1], cMqttValue, true);
	}

	// Fixed pattern
	else if (strcmp(topic, cMqttSetTopic[2]) == 0) {
		DEBUG("MQTT Fixed pattern: ");
		DEBUGLN(cMqttValue);		
		
		// Clear Fixed pattern value
		if (atoi(cMqttValue) > 0) {
			mqttClient.publish(cMqttGetTopic[1], "0", true);
		}

		if (atoi(cMqttValue) > 0) {
			beLedPattern = atoi(cMqttValue);

			FastLED.clear();

			changeLedFixedPatternParameters(beLedPattern);
		}

		// Publish new value
		mqttClient.publish(cMqttGetTopic[2], cMqttValue, true);
	}

	// Pattern color
	else if (strcmp(topic, cMqttSetTopic[3]) == 0) {
		DEBUG("MQTT Pattern color: ");
		DEBUGLN(cMqttValue);

		beLedColorPattern = atoi(cMqttValue);
		
		FastLED.clear();
		changeLedColorPatternParameters(beLedColorPattern);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[3], cMqttValue, true);
	}

	// Color
	else if (strcmp(topic, cMqttSetTopic[4]) == 0) {
		DEBUG("MQTT Color: ");
		DEBUGLN(cMqttValue);

		char cValue[16];

		sprintf(cValue, "%s", cMqttValue);

		char* cColors = strtok(cMqttValue, ",");

		byte i = 0;

		while (cColors != NULL)
		{
			switch (i) {
				// Red
				case 0:
					stLedColors.beRed = atoi(cColors);
					break;
					// Green
				case 1:
					stLedColors.beGreen = atoi(cColors);
					break;
					// Blue
				case 2:
					stLedColors.beBlue = atoi(cColors);
					break;
			}
			i++;
			cColors = strtok(NULL, ",");
		}

		crgbLedColors = CRGB(stLedColors.beRed, stLedColors.beGreen, stLedColors.beBlue);
		FastLED.clear();

		if (beLedPattern == 0) {
			setLedsColor(0, iNrOfLeds, crgbLedColors);
			FastLED.show();
		}

		// Publish new value
		mqttClient.publish(cMqttGetTopic[4], cValue, true);

		sLedHexColor = rgbToHex(stLedColors.beRed, stLedColors.beGreen, stLedColors.beBlue);
	}

	// Speed
	else if (strcmp(topic, cMqttSetTopic[5]) == 0) {
		DEBUG("MQTT Speed: ");
		DEBUGLN(cMqttValue);

		beLedSpeed = atoi(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[5], cMqttValue, true);
	}

	// Manual Display On
	else if (strcmp(topic, cMqttSetTopic[6]) == 0) {
		DEBUG("MQTT Manual Display On: ");
		DEBUGLN(cMqttValue);

		bBacklight = atoi(cMqttValue);
		DEBUGLN(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[6], cMqttValue, true);
	}

	// Automatic Display On enabled
	else if (strcmp(topic, cMqttSetTopic[7]) == 0) {
		DEBUG("MQTT Automatic Display On enabled: ");
		DEBUGLN(cMqttValue);

		bAutDisplayEnabled = atoi(cMqttValue);
		DEBUGLN(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[7], cMqttValue, true);
	}

	// Display On hour
	else if (strcmp(topic, cMqttSetTopic[8]) == 0) {
		DEBUG("MQTT Display On hour: ");
		DEBUGLN(cMqttValue);

		beDisplayOn = atoi(cMqttValue);
		DEBUGLN(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[8], cMqttValue, true);
	}

	// Display Off hour
	else if (strcmp(topic, cMqttSetTopic[9]) == 0) {
		DEBUG("MQTT Display Off hour: ");
		DEBUGLN(cMqttValue);

		beDisplayOff = atoi(cMqttValue);
		DEBUGLN(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[9], cMqttValue, true);
	}

	// PIR sensor enabled
	else if (strcmp(topic, cMqttSetTopic[10]) == 0) {
		DEBUG("MQTT PIR sensor enabled: ");
		DEBUGLN(cMqttValue);

		bPirSensorEnabled = atoi(cMqttValue);
		DEBUGLN(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[10], cMqttValue, true);
	}

	DEBUGLN("--- mqttCallback - End ---");
}

// Connect to server
void mqttConnect() {
	DEBUGLN("--- mqttConnect - Start ---");

	byte beMqttCounter = 0;

	// Try to create connection
	while (!mqttClient.connected()) {
		DEBUGLN("Attempting MQTT connection...");

		// Conect to MQTT broker with Hostname, User name, Password
		if (mqttClient.connect(cMqttClientId, cMqttUserName, cMqttUserPassword)) {
			DEBUGLN("connected");

			// Subscribe MQTT topics
			for (int i = 0; i < sizeof cMqttSetTopic / sizeof cMqttSetTopic[0]; i++) {
				mqttClient.subscribe(cMqttSetTopic[i]);
				DEBUG("Subscribbed to: ");
				DEBUGLN(cMqttSetTopic[i]);
			}

			// Send status data to MQTT broker
			//mqttSendStatus((void *)0);
		}
		else {
			DEBUG("failed, rc=");
			DEBUG(mqttClient.state());
			DEBUGLN(" try again in 3 seconds");
			// Wait 3 seconds before retrying
			delay(3000);
			beMqttCounter++;
		}

		// If counter exceed limit, stop MQTT connection
		if (beMqttCounter > 5) {
			bMqttConnError = true;
			break;
		}
	}

	DEBUGLN("--- mqttConnect - End ---");
}

// Check for MQTT message
void mqttCheckMessage(void *context) {
	mqttClient.loop();
}

// Send MQTT status
void mqttSendStatus(void *context) {
	DEBUGLN("--- mqttSendStatus - Start ---");

	for (int i = 0; i < sizeof cMqttGetTopic / sizeof cMqttGetTopic[0]; i++) {
		char cMqttValue[21];

		switch (i) {
			// Brightness
			case 0:
				sprintf(cMqttValue, "%i", beLedBrightness);
				break;

			// Pattern
			case 1:
				sprintf(cMqttValue, "%i", beLedPattern);
				break;

			// Fixed pattern
			case 2:
				sprintf(cMqttValue, "%i", beLedPattern);
				break;

			// Pattern color
			case 3:
				sprintf(cMqttValue, "%i", beLedColorPattern);
				break;

			// Color
			case 4:
				sprintf(cMqttValue, "%i,%i,%i", stLedColors.beRed, stLedColors.beGreen, stLedColors.beBlue);
				break;

			// Speed
			case 5:
				sprintf(cMqttValue, "%i", beLedSpeed);
				break;

			// Manual Display On
			case 6:
				sprintf(cMqttValue, "%i", bBacklight);
				break;

			// Automatic Display On enabled
			case 7:
				sprintf(cMqttValue, "%i", bAutDisplayEnabled);
				break;

			// Display On hour
			case 8:
				sprintf(cMqttValue, "%i", beDisplayOn);
				break;

			// Display Off hour
			case 9:
				sprintf(cMqttValue, "%i", beDisplayOff);
				break;

			// PIR sensor enabled
			case 10:
				sprintf(cMqttValue, "%i", bPirSensorEnabled);
				break;
		}

		// Publish new value
		mqttClient.publish(cMqttGetTopic[i], cMqttValue, true);

		DEBUG("Published to: ");
		DEBUGLN(cMqttGetTopic[i]);
		DEBUG("Value: ");
		DEBUGLN(cMqttValue);
	}

	DEBUGLN("--- mqttSendStatus - Start ---");
}

// Stadby mode
void stadbyMode() {
	turnOffLeds(0, iNrOfLeds);
	crgbLeds[1] = CRGB::Red;
	beLedBrightness = STANDBY_LED_BRIGHTNESS;
}

// PIR sensor
void getPirSensorState(uint8_t uiPirPinState) {
	if (uiPirPinState == HIGH) {
		if (bPirLockLow) {
			// Makes sure we wait for a transition to LOW before any further output is made
			bPirLockLow = false;

			bPirBacklightOn = true;

			DEBUGLN("Motion detected.");
		}
		bPirTakeLowTime = true;
	}

	if (uiPirPinState == LOW) {
		if (bPirTakeLowTime) {
			iPirLowIn = millis();          // save the time of the transition from high to LOW
			bPirTakeLowTime = false;       // Make sure this is only done at the start of a LOW phase
		}

		// If the sensor is low for more than the given iPirPause, we assume that no more motion is going to happen
		if (!bPirLockLow && millis() - iPirLowIn > iPirPause) {
			// Makes sure this block of code is only executed again after  a new motion sequence has been detected
			bPirLockLow = true;

			bPirBacklightOn = false;

			DEBUGLN("Motion ended.");
		}
	}
}
// ----------------------------------------------------
