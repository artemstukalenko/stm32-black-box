#include "MockGPS.h"
#include <stdio.h>
#include <stdlib.h>

MockGPS::MockGPS() : latitude(37.7749), longitude(-122.4194), altitude(10.0f) {
	stringBuffer[0] = '\0';
}

bool MockGPS::init() {
	return true;
}

void MockGPS::update() {
	latitude  = -90.0  + static_cast<double>(rand()) / RAND_MAX * 180.0;
	longitude = -180.0 + static_cast<double>(rand()) / RAND_MAX * 360.0;
	altitude  = static_cast<float>(rand() % 10000);

	snprintf(stringBuffer, sizeof(stringBuffer), "GPS -> LAT: %d LON: %d AL: %d\r\n",
			(int) latitude, (int) longitude, (int) altitude);
}

const char* MockGPS::getDataString() {
	return stringBuffer;
}

const char* MockGPS::getName() {
	return "MockGPS";
}

uint32_t MockGPS::getDelay() {
	return (uint32_t) 5000;
}
