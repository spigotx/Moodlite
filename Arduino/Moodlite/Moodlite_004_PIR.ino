// --------------------- PIR functions -----------------------

// PIR sensor
void getPirSensorState(uint8_t uiPirPinState) {
	if (uiPirPinState == HIGH) {
		if (bPirLockLow) {
			// Makes sure we wait for a transition to LOW before any further output is made
			bPirLockLow = false;
			bPirLockHigh = false;
			bPirDisplayOn = true;
			iPirHighIn = millis();          // Save the time of the transition from LOW to HIGH

			DEBUGLN("Motion detected.");
		}
		bPirTakeLowTime = true;
	}
	else {

		if (bPirTakeLowTime) {
			iPirLowIn = millis();          // Save the time of the transition from HIGH to LOW
			bPirTakeLowTime = false;       // Make sure this is only done at the start of a LOW phase
		}

		// If the sensor is LOW for more than the given iPirPause, we assume that no more motion is going to happen
		if (!bPirLockLow && millis() - iPirLowIn > iPirPause) {
			// Makes sure this block of code is only executed again after  a new motion sequence has been detected
			bPirLockLow = true;

			//bPirDisplayOn = false;

			DEBUGLN("Motion ended.");
		}

		// If time bePirDisplayOnTime (converted minutes to seconds) passed since the sensor was HIGH, turn Off the LEDs
		if (!bPirLockHigh && millis() - iPirHighIn > (bePirDisplayOnTime * 60000)) {
			// Makes sure this block of code is only executed again after a new motion sequence has been detected
			bPirLockHigh = true;
			bPirDisplayOn = false;

			DEBUGLN("LEDs Off.");
		}
	}
}

// -----------------------------------------------------------