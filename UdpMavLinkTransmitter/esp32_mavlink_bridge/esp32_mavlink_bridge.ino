/**
 * esp32_mavlink_bridge.ino
 *
 * Dumb UART<->UDP bridge for UAV_BlackBox telemetry.
 * The STM32 side already frames complete MAVLink v2 packets
 * (see MavLinkPacketBuilder), so this sketch does NOT parse MAVLink -
 * it just forwards raw bytes in both directions.
 *
 * Downlink : STM32 UART -> UDP (broadcast until a GCS is seen, then unicast)
 * Uplink   : UDP (from QGC)   -> STM32 UART
 *
 * Board: ESP32 DevKit (LunaNode32), 3.3V logic, no level shifter needed.
 */

#include <WiFi.h>
#include <WiFiUdp.h>

// ---- Wi-Fi AP config -------------------------------------------------
const char* AP_SSID     = "UAV_BlackBox";
const char* AP_PASSWORD = "changeme123";   // 8+ chars, WPA2

// ---- UDP config --------------------------------------------------------
// 14550 is QGroundControl's default auto-connect UDP port.
const uint16_t MAVLINK_UDP_PORT = 14550;
WiFiUDP udp;

IPAddress gcsIp;              // learned from the first packet QGC sends us
bool gcsKnown = false;

#define STM32_SERIAL Serial2
const uint32_t STM32_BAUD = 115200;

// Buffer for one UART->UDP forwarding burst
uint8_t uartBuf[512];

void setup() {
	Serial.begin(115200);          // USB debug console
	STM32_SERIAL.begin(STM32_BAUD, SERIAL_8N1, 16, 17);

	WiFi.mode(WIFI_AP);
	WiFi.softAP(AP_SSID, AP_PASSWORD);
	Serial.print("AP started, IP: ");
	Serial.println(WiFi.softAPIP());

	udp.begin(MAVLINK_UDP_PORT);
}

void loop() {
	forwardUartToUdp();
	forwardUdpToUart();
}

/** STM32 -> QGC */
void forwardUartToUdp() {
	size_t n = 0;
	// Drain whatever's currently buffered, up to our buffer size.
	while (STM32_SERIAL.available() && n < sizeof(uartBuf)) {
		uartBuf[n++] = STM32_SERIAL.read();
	}
	if (n == 0) {
		return;
	}

	if (gcsKnown) {
		udp.beginPacket(gcsIp, MAVLINK_UDP_PORT);
	} else {
		// No GCS has talked to us yet - broadcast so QGC's auto-connect
		// (which listens on 14550) can discover us with zero config.
		udp.beginPacket(IPAddress(255, 255, 255, 255), MAVLINK_UDP_PORT);
	}

	udp.write(uartBuf, n);
	udp.endPacket();
}

/** QGC -> STM32 */
void forwardUdpToUart() {
	int packetSize = udp.parsePacket();
	if (packetSize <= 0) {
		return;
	}

	// First packet we see from a client tells us who to unicast to -
	// avoids flooding broadcast traffic once QGC is actually connected.
	gcsIp = udp.remoteIP();
	gcsKnown = true;

	uint8_t rxBuf[512];
	int len = udp.read(rxBuf, sizeof(rxBuf));
	if (len > 0) {
		STM32_SERIAL.write(rxBuf, len);
	}
}
