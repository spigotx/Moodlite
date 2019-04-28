// ------------------- System functions ----------------------

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

	if (WiFi.status() == WL_CONNECTED) {
		DEBUGLN("Wifi network started succesfully.");

		DEBUG("IP address: ");
		DEBUGLN(WiFi.localIP());
	}
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

	//ESP.restart();
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

		// Number of sides per tile
		char cConfiguredNrOfSides[40] = "";
		readCharFromEEPROM(cConfiguredNrOfSides, NUMBER_OF_SIDES_PER_TILE_START, NUMBER_OF_SIDES_PER_TILE_MAX);

		if (strlen(cConfiguredNrOfSides) > 0) {
			sprintf(cNrOfSidesPerTile, "%s", cConfiguredNrOfSides);

			beNrOfSidesPerTileSize = setArraySidesPerTile(cNrOfSidesPerTile);
		}

		// LED Color
		sLedHexColor = readStringFromEEPROM(LED_COLOR_START, LED_COLOR_MAX);
		stLedColors = hexToRGB(sLedHexColor);
		crgbLedColors = CRGB(stLedColors.beRed, stLedColors.beGreen, stLedColors.beBlue);

		// Display mode
		bDisplayMode = readByteFromEEPROM(DISPLAY_MODE_START);

		// Manual Display On
		bBacklight = readByteFromEEPROM(BACKLIGHT_START);

		// Automatic Display On
		bAutDisplayEnabled = readByteFromEEPROM(AUTDISPLAYON_START);
		beDisplayOn = readByteFromEEPROM(DISPLAYON_START);
		beDisplayOff = readByteFromEEPROM(DISPLAYOFF_START);

		// PIR senor (motion sensor) enabled
		bPirSensorEnabled = readByteFromEEPROM(PIR_SENSOR_ENABLED_START);

		// PIR senor (motion sensor) Display On time
		bePirDisplayOnTime = readByteFromEEPROM(PIR_SENSOR_DISPLAY_ON_TIME_START);

		// LED Brightness
		beLedBrightness = beLedOldBrightness = readByteFromEEPROM(LED_BRIGHTNESS_START);

		// LED Speed
		beLedSpeed = readByteFromEEPROM(LED_SPEED_START);

		//LED Pattern
		beLedPattern = readByteFromEEPROM(LED_PATTERN_START);

		//LED Individual effect
		readCharFromEEPROM(cLedEffect, LED_EFFECT_START, LED_EFFECT_MAX);
		iLedEffect = atoi(cLedEffect);

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
		
		// Timezone rules
		// Daylight TZ
		// Abbreviation 
		readCharFromEEPROM(cTzDstRuleTz, TIMEZONE_DST_ABBREV_START, TIMEZONE_DST_ABBREV_MAX);
		strcpy(tchrDst.abbrev, cTzDstRuleTz);
		// Week
		beTzDstRuleWeek = readByteFromEEPROM(TIMEZONE_DST_WEEK_START);
		tchrDst.week = beTzDstRuleWeek;
		sprintf(cTzDstRuleWeek, "%i", beTzDstRuleWeek);
		// Day of week
		beTzDstRuleDow = readByteFromEEPROM(TIMEZONE_DST_DOW_START);
		tchrDst.dow = beTzDstRuleDow;
		sprintf(cTzDstRuleDow, "%i", beTzDstRuleDow);	
		// Month
		beTzDstRuleMonth = readByteFromEEPROM(TIMEZONE_DST_MONTH_START);
		tchrDst.month = beTzDstRuleMonth;
		sprintf(cTzDstRuleMonth, "%i", beTzDstRuleMonth);		
		// Hour
		beTzDstRuleHour = readByteFromEEPROM(TIMEZONE_DST_HOUR_START);
		tchrDst.hour = beTzDstRuleHour;
		sprintf(cTzDstRuleHour, "%i", beTzDstRuleHour);	
		// Offset
		readCharFromEEPROM(cTzDstRuleOffset, TIMEZONE_DST_OFFSET_START, TIMEZONE_DST_OFFSET_MAX);
		iTzDstRuleOffset = atoi(cTzDstRuleOffset);
		tchrDst.offset = iTzDstRuleOffset;
		// Standard TZ
		// Abbreviation 
		readCharFromEEPROM(cTzStdRuleTz, TIMEZONE_STD_ABBREV_START, TIMEZONE_STD_ABBREV_MAX);
		strcpy(tchrStd.abbrev, cTzStdRuleTz);
		// Week
		beTzStdRuleWeek = readByteFromEEPROM(TIMEZONE_STD_WEEK_START);
		tchrStd.week = beTzStdRuleWeek;
		sprintf(cTzStdRuleWeek, "%i", beTzStdRuleWeek);
		// Day of week
		beTzStdRuleDow = readByteFromEEPROM(TIMEZONE_STD_DOW_START);
		tchrStd.dow = beTzStdRuleDow;
		sprintf(cTzStdRuleDow, "%i", beTzStdRuleDow);
		// Month
		beTzStdRuleMonth = readByteFromEEPROM(TIMEZONE_STD_MONTH_START);
		tchrStd.month = beTzStdRuleMonth;
		sprintf(cTzStdRuleMonth, "%i", beTzStdRuleMonth);
		// Hour
		beTzStdRuleHour = readByteFromEEPROM(TIMEZONE_STD_HOUR_START);
		tchrStd.hour = beTzStdRuleHour;
		sprintf(cTzStdRuleHour, "%i", beTzStdRuleHour);
		// Offset
		readCharFromEEPROM(cTzStdRuleOffset, TIMEZONE_STD_OFFSET_START, TIMEZONE_STD_OFFSET_MAX);
		iTzStdRuleOffset = atoi(cTzStdRuleOffset);
		tchrStd.offset = iTzStdRuleOffset;

		// Update Timezone
		tz.setRules(tchrDst, tchrStd);
		
		DEBUG("Display mode: ");
		DEBUGLN(bDisplayMode);
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
		DEBUG("Pir sensor Display On time: ");
		DEBUGLN(bePirDisplayOnTime);
		DEBUG("Nr. of LEDs: ");
		DEBUGLN(iNrOfLeds);
		DEBUG("Nr. of LEDs per corner: ");
		DEBUGLN(beNrOfLedsCorner);
		DEBUG("Number of tiles:");
		DEBUGLN(beNrOfSidesPerTileSize);
		DEBUG("Nr. of sides per tile: ");
		DEBUGLN(cNrOfSidesPerTile);
		DEBUG("Brightness: ");
		DEBUGLN(beLedBrightness);
		DEBUG("Speed: ");
		DEBUGLN(beLedSpeed);
		DEBUG("Pattern: ");
		DEBUGLN(beLedPattern);
        DEBUG("Pattern color: ");
        DEBUGLN(beLedColorPattern);
		DEBUG("Individual effect: ");
		DEBUGLN(iLedEffect);
		DEBUG("LED color: ");
		DEBUGLN(sLedHexColor);
		DEBUGLN("Timezone DST rules: ");
		DEBUG("Abbrevation: ");
		DEBUGLN(cTzDstRuleTz);
		DEBUG("Week: ");
		DEBUGLN(beTzDstRuleWeek);
		DEBUG("Day of week: ");
		DEBUGLN(beTzDstRuleDow);
		DEBUG("Month: ");
		DEBUGLN(beTzDstRuleMonth);
		DEBUG("Hour: ");
		DEBUGLN(beTzDstRuleHour);
		DEBUG("Offset: ");
		DEBUGLN(cTzDstRuleOffset);
		DEBUGLN("Timezone STD rules: ");
		DEBUG("Abbrevation: ");
		DEBUGLN(cTzStdRuleTz);
		DEBUG("Week: ");
		DEBUGLN(beTzStdRuleWeek);
		DEBUG("Day of week: ");
		DEBUGLN(beTzStdRuleDow);
		DEBUG("Month: ");
		DEBUGLN(beTzStdRuleMonth);
		DEBUG("Hour: ");
		DEBUGLN(beTzStdRuleHour);
		DEBUG("Offset: ");
		DEBUGLN(cTzStdRuleOffset);
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
			setDefaultValues();
			ESP.restart();
		}
	#endif
}

// Set default values to EEPROM
void setDefaultValues() {
	DEBUGLN("--- setDefaultValues - Start ---");

	writeByteToEEPROM(1, BACKLIGHT_START);
	writeByteToEEPROM(20, LED_BRIGHTNESS_START);
	writeCharToEEPROM("FFFFFF", LED_COLOR_START);
	writeByteToEEPROM(1, NUMBER_OF_LEDS_CORNER_START);
	writeCharToEEPROM("3", NUMBER_OF_LEDS_START);
	writeByteToEEPROM(50, LED_SPEED_START);
	writeCharToEEPROM("3,", NUMBER_OF_SIDES_PER_TILE_START);
	
	DEBUGLN("--- setDefaultValues - End ---");
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
		if ((bAutDisplayEnabled && checkHour()) || (!bAutDisplayEnabled && bBacklight) || (bPirDisplayOn)) {
			// On change
			// beLedBrightness == STANDBY_LED_BRIGHTNESS -> Standby mode
			if (beLedBrightness == STANDBY_LED_BRIGHTNESS) {
				beLedBrightness = beLedOldBrightness;

				// Update Web interface
				broadcastUpdate("led_strip_brightness", String(beLedBrightness));
			}
		}
		else {
			// On change
			if (beLedBrightness > STANDBY_LED_BRIGHTNESS) {
				beLedOldBrightness = beLedBrightness;

				// Update Web interface
				broadcastUpdate("led_strip_brightness", String(beLedBrightness));
			}

			standbyMode();

			bRet = false;
		}

		// Execute only if device is configured
		if (strlen(cWifiSsid) > 0) {
			// Change Rgb LED brightness
			FastLED.setBrightness(beLedBrightness);
		}
	}

	return(bRet);
}

// -----------------------------------------------------------
