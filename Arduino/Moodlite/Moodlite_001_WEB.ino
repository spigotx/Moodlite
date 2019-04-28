// ---------------- Web interface functions ------------------

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

		case 4:
			sendTimeValues(client);
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
		"\"led_strip_nr_of_leds_corner\":" + beNrOfLedsCorner + "," +
		"\"led_strip_nr_of_leds\":" + iNrOfLeds + "," +
		"\"led_strip_nr_sides_tile\":\"" + cNrOfSidesPerTile + "\"," +
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
		"\"effects\":" + iLedEffect + "," +
		"\"display_mode\":" + bDisplayMode + "," +
		"\"aut_display_enabled\":" + bAutDisplayEnabled + "," +
		"\"display_on\":" + beDisplayOn + "," +
		"\"display_off\":" + beDisplayOff + "," +
		"\"backlight\":" + bBacklight + "," +
		"\"pir_sensor_endabled\":" + bPirSensorEnabled + "," +
		"\"pir_sensor_display_on_time\":" + bePirDisplayOnTime + "," +
		"\"colorpickerfield_led_strip\":\"" + sLedHexColor + "\"," +
		"\"led_strip_brightness\":" + beLedBrightness + "," +
		"\"led_strip_speed\":" + beLedSpeed +
		"}}";

	client->text(json);
	
	DEBUGLN("--- sendLedValues - End ---");
}

void sendTimeValues(AsyncWebSocketClient *client) {
	DEBUGLN("--- sendTimeValues - Start ---");

	String sNtpDateTime = String(hour()) + " : " + String(minute()) + " : " + String(second());

	String json = String("{\"type\":\"sv.init.settings\",\"value\":{") +
		"\"tz_dst_rule_tz\":\"" + cTzDstRuleTz + "\"," +
		"\"tz_dst_rule_week\":" + beTzDstRuleWeek + "," +
		"\"tz_dst_rule_dow\":" + beTzDstRuleDow + "," +
		"\"tz_dst_rule_month\":" + beTzDstRuleMonth + "," +
		"\"tz_dst_rule_hour\":" + beTzDstRuleHour + "," +
		"\"tz_dst_rule_offset\":" + iTzDstRuleOffset + "," +
		"\"tz_std_rule_tz\":\"" + cTzStdRuleTz + "\"," +
		"\"tz_std_rule_week\":" + beTzStdRuleWeek + "," +
		"\"tz_std_rule_dow\":" + beTzStdRuleDow + "," +
		"\"tz_std_rule_month\":" + beTzStdRuleMonth + "," +
		"\"tz_std_rule_hour\":" + beTzStdRuleHour + "," +
		"\"tz_std_rule_offset\":" + iTzStdRuleOffset + "," +
		"\"ntp_server\":\"" + sNtpServer + "\"," +
		"\"ntp_date_time\":\"" + sNtpDateTime + "\"" +
		"}}";

	client->text(json);
	
	DEBUGLN("--- sendTimeValues - End ---");
}

// Update Web interface value
void broadcastUpdate(String field, String value) {
	DEBUGLN("--- broadcastUpdate - Start ---");

	String json = String("{\"type\":\"sv.update\",\"value\":{") + "\"" + field + "\":" + value + "}}";

	ayncWebSocket.textAll(json);

	DEBUGLN("--- broadcastUpdate - End ---");
}

// Update Web interface string value
void broadcastUpdateWithQuotes(String field, String value) {
	DEBUGLN("--- broadcastUpdateWithQuotes - Start ---");

	String json = String("{\"type\":\"sv.update\",\"value\":{") + "\"" + field + "\":\"" + value + "\"}}";

	ayncWebSocket.textAll(json);

	DEBUGLN("--- broadcastUpdateWithQuotes - End ---");
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
	else if (strcmp("pir_sensor_endabled", key) == 0) {
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


	// PIR sensor Display On time
	else if (strcmp("pir_sensor_display_on_time", key) == 0) {
		DEBUG("pir_sensor_display_on_time: ");
		DEBUGLN(value);

		bePirDisplayOnTime = value.toInt();

		DEBUGLN(bePirDisplayOnTime);

		// Publish new value
		sprintf(cMqttValue, "%i", bePirDisplayOnTime);
		mqttClient.publish(cMqttGetTopic[11], cMqttValue, true);
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

		// Turn Off all LEDs
		FastLED.clear();

		if (beLedPattern == 0) {
			setLedsColor(0, iNrOfLeds, crgbLedColors);
		}

		FastLED.show();

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

	// Display mode
	// True = Tile mode
	// False = LED mode
	else if (strcmp("display_mode", key) == 0) {
		DEBUG("display_mode: ");
		DEBUGLN(value);

		// Convert string value "true"/"false" to boolean for further calculation
		const char *to_find = value.c_str(); // string we want to find

		if (strcmp(to_find, "true") == 0) { // this is the key: a match returns 0
			bDisplayMode = true;
		}
		else {
			bDisplayMode = false;
		}

		// Publish new value
		sprintf(cMqttValue, "%i", bDisplayMode);
		mqttClient.publish(cMqttGetTopic[13], cMqttValue, true);
	}

	// LED pattern color
	else if (strcmp("patterncolor", key) == 0) {
		DEBUG("patterncolor: ");
		DEBUGLN(value);

		beLedColorPattern = value.toInt();

		FastLED.clear();
		FastLED.show();

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
		FastLED.show();

		changeLedPatternParameters(beLedPattern);

		DEBUGLN(beLedPattern);

		// MQTT
		// Clear Fixed pattern value
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
		FastLED.show();

        changeLedFixedPatternParameters(beLedPattern);

        DEBUGLN(beLedPattern);

        // MQTT
        // Clear Pattern value
        sprintf(cMqttValue, "%i", 0);
        mqttClient.publish(cMqttGetTopic[1], cMqttValue, true);
        // Clear Pattern color value
        mqttClient.publish(cMqttGetTopic[3], cMqttValue, true);
        // Publish new values
        sprintf(cMqttValue, "%i", beLedPattern);
        mqttClient.publish(cMqttGetTopic[2], cMqttValue, true);
    }

   // LED effects
    else if (strcmp("effects", key) == 0) {
        DEBUG("Individual effects: ");
        DEBUGLN(value);
		
		value.toCharArray(cLedEffect, 5);

        iLedEffect = value.toInt();

        DEBUGLN(iLedEffect);

		// MQTT
		// Publish new values
		sprintf(cMqttValue, "%i", iLedEffect);
		mqttClient.publish(cMqttGetTopic[12], cMqttValue, true);
    }

	// Store as default values
	else if (strcmp("saveDefaultValues", key) == 0) {
		DEBUG("saveDefaultValues: ");
		DEBUGLN(value);

		if (value.toInt() == 1) {
			DEBUGLN("Store data");
			DEBUG("Display mode: ");
			DEBUGLN(bDisplayMode);
			writeByteToEEPROM((int)bDisplayMode, DISPLAY_MODE_START);
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
			DEBUG("PIR sensor enabled: ");
			DEBUGLN(bPirSensorEnabled);
			writeByteToEEPROM((int)bPirSensorEnabled, PIR_SENSOR_ENABLED_START);
			DEBUG("PIR sensor Display On time: ");
			DEBUGLN(bePirDisplayOnTime);
			writeByteToEEPROM(bePirDisplayOnTime, PIR_SENSOR_DISPLAY_ON_TIME_START);
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
			DEBUG("Individual effect: ");
			DEBUGLN(cLedEffect);
			writeCharToEEPROM(cLedEffect, LED_EFFECT_START);
		}
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

	// Set total sides per tile
	else if (strcmp("setNrOfSidesPerTile", key) == 0) {
		DEBUG("setNrOfSidesPerTile: ");
		DEBUGLN(value);

		value.toCharArray(cNrOfSidesPerTile, 40);

		setArraySidesPerTile(cNrOfSidesPerTile);

		DEBUGLN(cNrOfSidesPerTile);
	}

	// Save LEDs settings
	else if (strcmp("saveLedsSettings", key) == 0) {
		DEBUG("saveLedsSettings: ");
		DEBUGLN(value);

		if (value.toInt() == 1) {
			writeByteToEEPROM(beNrOfLedsCorner, NUMBER_OF_LEDS_CORNER_START);
			writeStringToEEPROM(sNrOfLeds, NUMBER_OF_LEDS_START, NUMBER_OF_LEDS_MAX);
			writeCharToEEPROM(cNrOfSidesPerTile, NUMBER_OF_SIDES_PER_TILE_START);		
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

	// Daylight time
	// Time zone
	else if (strcmp("tz_dst_rule_tz", key) == 0) {
		DEBUG("tz_dst_rule_tz: ");
		DEBUGLN(value);

		value.toCharArray(cTzDstRuleTz, 6);
		strcpy(tchrDst.abbrev, cTzDstRuleTz);

		DEBUGLN(tchrDst.abbrev);
	}

	// Week
	else if (strcmp("tz_dst_rule_week", key) == 0) {
		DEBUG("tz_dst_rule_week: ");
		DEBUGLN(value);

		beTzDstRuleWeek = value.toInt();
		sprintf(cTzDstRuleWeek, "%i", beTzDstRuleWeek);
		tchrDst.week = beTzDstRuleWeek;

		DEBUGLN(tchrDst.week);
	}

	// Day of week
	else if (strcmp("tz_dst_rule_dow", key) == 0) {
		DEBUG("tz_dst_rule_dow: ");
		DEBUGLN(value);

		beTzDstRuleDow = value.toInt();
		sprintf(cTzDstRuleDow, "%i", beTzDstRuleDow);
		tchrDst.dow = beTzDstRuleDow;

		DEBUGLN(tchrDst.dow);
	}

	// Month
	else if (strcmp("tz_dst_rule_month", key) == 0) {
		DEBUG("tz_dst_rule_month: ");
		DEBUGLN(value);

		beTzDstRuleMonth = value.toInt();
		sprintf(cTzDstRuleMonth, "%i", beTzDstRuleMonth);
		tchrDst.month = beTzDstRuleMonth;

		DEBUGLN(tchrDst.month);
	}

	// Hour
	else if (strcmp("tz_dst_rule_hour", key) == 0) {
		DEBUG("tz_dst_rule_hour: ");
		DEBUGLN(value);

		beTzDstRuleHour = value.toInt();
		sprintf(cTzDstRuleHour, "%i", beTzDstRuleHour);
		tchrDst.hour = beTzDstRuleHour;

		DEBUGLN(tchrDst.hour);
	}

	// Offset
	else if (strcmp("tz_dst_rule_offset", key) == 0) {
		DEBUG("tz_dst_rule_offset: ");
		DEBUGLN(value);

		value.toCharArray(cTzDstRuleOffset, 6);
		iTzDstRuleOffset = value.toInt();
		tchrDst.offset = iTzDstRuleOffset;

		DEBUGLN(tchrDst.offset);
	}

	// Standard time
	// Time zone
	else if (strcmp("tz_std_rule_tz", key) == 0) {
		DEBUG("tz_std_rule_tz: ");
		DEBUGLN(value);

		value.toCharArray(cTzStdRuleTz, 6);
		strcpy(tchrStd.abbrev, cTzStdRuleTz);

		DEBUGLN(tchrStd.abbrev);
	}

	// Week
	else if (strcmp("tz_std_rule_week", key) == 0) {
		DEBUG("tz_std_rule_week: ");
		DEBUGLN(value);

		beTzStdRuleWeek = value.toInt();
		sprintf(cTzDstRuleWeek, "%i", beTzStdRuleWeek);
		tchrStd.week = beTzStdRuleWeek;

		DEBUGLN(tchrStd.week);
	}

	// Day of week
	else if (strcmp("tz_std_rule_dow", key) == 0) {
		DEBUG("tz_std_rule_dow: ");
		DEBUGLN(value);

		beTzStdRuleDow = value.toInt();
		sprintf(cTzDstRuleDow, "%i", beTzStdRuleDow);
		tchrStd.dow = beTzStdRuleDow;

		DEBUGLN(tchrStd.dow);
	}

	// Month
	else if (strcmp("tz_std_rule_month", key) == 0) {
		DEBUG("tz_std_rule_month: ");
		DEBUGLN(value);

		beTzStdRuleMonth = value.toInt();
		sprintf(cTzDstRuleMonth, "%i", beTzStdRuleMonth);
		tchrStd.month = beTzStdRuleMonth;

		DEBUGLN(tchrStd.month);
	}

	// Hour
	else if (strcmp("tz_std_rule_hour", key) == 0) {
		DEBUG("tz_std_rule_hour: ");
		DEBUGLN(value);

		beTzStdRuleHour = value.toInt();
		sprintf(cTzDstRuleHour, "%i", beTzStdRuleHour);
		tchrStd.hour = beTzStdRuleHour;

		DEBUGLN(tchrStd.hour);
	}

	// Offset
	else if (strcmp("tz_std_rule_offset", key) == 0) {
		DEBUG("tz_std_rule_offset: ");
		DEBUGLN(value);

		value.toCharArray(cTzStdRuleOffset, 6);
		iTzStdRuleOffset = value.toInt();
		tchrStd.offset = iTzStdRuleOffset;

		DEBUGLN(tchrStd.offset);
	}

	// Set NTP server
	else if (strcmp("setNtpServer", key) == 0) {
		DEBUG("setNtpServer: ");
		DEBUGLN(value);

		sNtpServer = value;

		DEBUGLN(sNtpServer);
	}

	// Save time settings
	else if (strcmp("saveTimeSettings", key) == 0) {
		DEBUG("saveTimeSettings: ");
		DEBUGLN(value);

		if (value.toInt() == 1) {
			writeStringToEEPROM(sNtpServer, NTP_SERVER_START, NTP_SERVER_MAX);
			writeCharToEEPROM(cTzDstRuleTz, TIMEZONE_DST_ABBREV_START);
			writeByteToEEPROM(beTzDstRuleWeek, TIMEZONE_DST_WEEK_START);
			writeByteToEEPROM(beTzDstRuleDow, TIMEZONE_DST_DOW_START);
			writeByteToEEPROM(beTzDstRuleMonth, TIMEZONE_DST_MONTH_START);
			writeByteToEEPROM(beTzDstRuleHour, TIMEZONE_DST_HOUR_START);
			writeCharToEEPROM(cTzDstRuleOffset, TIMEZONE_DST_OFFSET_START);
			writeCharToEEPROM(cTzStdRuleTz, TIMEZONE_STD_ABBREV_START);
			writeByteToEEPROM(beTzStdRuleWeek, TIMEZONE_STD_WEEK_START);
			writeByteToEEPROM(beTzStdRuleDow, TIMEZONE_STD_DOW_START);
			writeByteToEEPROM(beTzStdRuleMonth, TIMEZONE_STD_MONTH_START);
			writeByteToEEPROM(beTzStdRuleHour, TIMEZONE_STD_HOUR_START);
			writeCharToEEPROM(cTzStdRuleOffset, TIMEZONE_STD_OFFSET_START);
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

	if ((beNrOfLedsCorner < 1) || (iNrOfLeds < 3)) {
		setDefaultValues();
	}
	
	DEBUGLN("--- setDefaulValues - End ---");

	ESP.restart();
}

// -----------------------------------------------------------
