#include "MockBarometer.h"
#include <stdio.h>
#include <stdlib.h>

MockBarometer::MockBarometer() : temperature(25.0f), pressure(101325.0f) {
	stringBuffer[0] = '\0';
}

bool MockBarometer::init() {
	return true;
}

void MockBarometer::update() {
	float tempDelta = ((float)(rand() % 10) - 5.0f) / 10.0f;
	float pressDelta = (float)(rand() % 100) - 50.0f;

	temperature += tempDelta;
	pressure += pressDelta;

	snprintf(stringBuffer, sizeof(stringBuffer), "BMP280 -> P: %d Pa, T: %d C\r\n",
			(int) pressure, (int) temperature);
}

const char* MockBarometer::getDataString() {
	return stringBuffer;
}

const char* MockBarometer::getName() {
	return "MockBarometer";
}

uint32_t MockBarometer::getDelay() {
	return (uint32_t) 2000;
}
