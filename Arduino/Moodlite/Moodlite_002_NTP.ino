// --------------------- NTP functions -----------------------

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

		// i > 5, stop sending packet and mark NTP packet error
		if (i > 5) {
			bNtpPacketError = true;
			break;
		}

		i++;
	}

	if (cbParsePacket) {
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

		localTime = tz.toLocal(utc, &tcr);

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

// -----------------------------------------------------------