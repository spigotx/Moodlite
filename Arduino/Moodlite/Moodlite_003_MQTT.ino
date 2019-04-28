// -------------------- MQTT functions -----------------------

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

	DEBUG("MQTT Message value: ");
	DEBUGLN(cMqttValue);

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

		// Update Web interface
		broadcastUpdate("led_strip_brightness", String(beLedBrightness));
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
			FastLED.show();

			changeLedPatternParameters(beLedPattern);
		}

		// Publish new value
		mqttClient.publish(cMqttGetTopic[1], cMqttValue, true);

		// Update Web interface
		broadcastUpdate("patterns", String(beLedPattern));
	}

	// Fixed pattern
	else if (strcmp(topic, cMqttSetTopic[2]) == 0) {
		DEBUG("MQTT Fixed pattern: ");
		DEBUGLN(cMqttValue);		
		
		// Clear values
		if (atoi(cMqttValue) > 0) {
			// Clear Pattern value
			mqttClient.publish(cMqttGetTopic[1], "0", true);
			// Clear Pattern color value
			mqttClient.publish(cMqttGetTopic[3], "0", true);
		}

		if (atoi(cMqttValue) > 0) {
			beLedPattern = atoi(cMqttValue);

			FastLED.clear();
			FastLED.show();

			changeLedFixedPatternParameters(beLedPattern);
		}

		// Publish new value
		mqttClient.publish(cMqttGetTopic[2], cMqttValue, true);

		// Update Web interface
		broadcastUpdate("fixed_patterns", String(beLedPattern));
	}

	// Pattern color
	else if (strcmp(topic, cMqttSetTopic[3]) == 0) {
		DEBUG("MQTT Pattern color: ");
		DEBUGLN(cMqttValue);

		beLedColorPattern = atoi(cMqttValue);
		
		FastLED.clear();
		FastLED.show();

		changeLedColorPatternParameters(beLedColorPattern);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[3], cMqttValue, true);

		// Update Web interface
		broadcastUpdate("pattern_color", String(beLedColorPattern));
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
		}

		FastLED.show();

		// Publish new value
		mqttClient.publish(cMqttGetTopic[4], cValue, true);

		sLedHexColor = rgbToHex(stLedColors.beRed, stLedColors.beGreen, stLedColors.beBlue);

		// Update Web interface
		broadcastUpdateWithQuotes("colorpickerfield_led_strip", String(sLedHexColor));
	}

	// Speed
	else if (strcmp(topic, cMqttSetTopic[5]) == 0) {
		DEBUG("MQTT Speed: ");
		DEBUGLN(cMqttValue);

		beLedSpeed = atoi(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[5], cMqttValue, true);

		// Update Web interface
		broadcastUpdate("led_strip_speed", String(beLedSpeed));
	}

	// Manual Display On
	else if (strcmp(topic, cMqttSetTopic[6]) == 0) {
		DEBUG("MQTT Manual Display On: ");
		DEBUGLN(cMqttValue);

		bBacklight = atoi(cMqttValue);
		DEBUGLN(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[6], cMqttValue, true);

		// Update Web interface
		broadcastUpdate("backlight", bBacklight ? "true" : "false");
	}

	// Automatic Display On enabled
	else if (strcmp(topic, cMqttSetTopic[7]) == 0) {
		DEBUG("MQTT Automatic Display On enabled: ");
		DEBUGLN(cMqttValue);

		bAutDisplayEnabled = atoi(cMqttValue);
		DEBUGLN(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[7], cMqttValue, true);

		// Update Web interface
		broadcastUpdate("aut_display_enabled", bAutDisplayEnabled ? "true" : "false");
	}

	// Display On hour
	else if (strcmp(topic, cMqttSetTopic[8]) == 0) {
		DEBUG("MQTT Display On hour: ");
		DEBUGLN(cMqttValue);

		beDisplayOn = atoi(cMqttValue);
		DEBUGLN(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[8], cMqttValue, true);

		// Update Web interface
		broadcastUpdate("display_on", String(beDisplayOn));
	}

	// Display Off hour
	else if (strcmp(topic, cMqttSetTopic[9]) == 0) {
		DEBUG("MQTT Display Off hour: ");
		DEBUGLN(cMqttValue);

		beDisplayOff = atoi(cMqttValue);
		DEBUGLN(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[9], cMqttValue, true);

		// Update Web interface
		broadcastUpdate("display_off", String(beDisplayOff));
	}

	// PIR sensor enabled
	else if (strcmp(topic, cMqttSetTopic[10]) == 0) {
		DEBUG("MQTT PIR sensor enabled: ");
		DEBUGLN(cMqttValue);

		bPirSensorEnabled = atoi(cMqttValue);
		DEBUGLN(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[10], cMqttValue, true);

		// Update Web interface
		broadcastUpdate("pir_sensor_endabled", bPirSensorEnabled ? "true" : "false");
	}

	// PIR sensor Display On time
	else if (strcmp(topic, cMqttSetTopic[11]) == 0) {
		DEBUG("MQTT PIR sensor Display On time: ");
		DEBUGLN(cMqttValue);

		bePirDisplayOnTime = atoi(cMqttValue);
		DEBUGLN(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[11], cMqttValue, true);

		// Update Web interface
		broadcastUpdate("pir_sensor_display_on_time", String(bePirDisplayOnTime));
	}

	// Individual effect
	else if (strcmp(topic, cMqttSetTopic[12]) == 0) {
		DEBUG("MQTT Individual effect: ");
		DEBUGLN(cMqttValue);

		strcpy(cLedEffect, cMqttValue);

		iLedEffect = atoi(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[12], cMqttValue, true);

		// Update Web interface
		broadcastUpdate("effects", String(iLedEffect));
	}

	// Display mode
	else if (strcmp(topic, cMqttSetTopic[13]) == 0) {
		DEBUG("MQTT Display mode: ");
		DEBUGLN(cMqttValue);

		bDisplayMode = atoi(cMqttValue);
		DEBUGLN(cMqttValue);

		// Publish new value
		mqttClient.publish(cMqttGetTopic[13], cMqttValue, true);

		// Update Web interface
		broadcastUpdate("display_mode", bDisplayMode ? "true" : "false");
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

			// Send status data to MQTT broker
			mqttSendStatus((void *)0);

			// Subscribe MQTT topics
			for (int i = 0; i < sizeof cMqttSetTopic / sizeof cMqttSetTopic[0]; i++) {
				mqttClient.subscribe(cMqttSetTopic[i]);
				DEBUG("Subscribbed to: ");
				DEBUGLN(cMqttSetTopic[i]);
			}
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
				if (beLedPattern < 99)
					sprintf(cMqttValue, "%i", beLedPattern);
				else
					sprintf(cMqttValue, "%i", 0);
				break;

			// Fixed pattern
			case 2:
				if (beLedPattern >= 100)
					sprintf(cMqttValue, "%i", beLedPattern);
				else
					sprintf(cMqttValue, "%i", 0);
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

			// PIR sensor Display On time
			case 11:
				sprintf(cMqttValue, "%i", bePirDisplayOnTime);
				break;

			// Individual effect
			case 12:
				sprintf(cMqttValue, "%i", iLedEffect);
				break;

			// Display mode
			case 13:
				sprintf(cMqttValue, "%i", bDisplayMode);
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

// -----------------------------------------------------------