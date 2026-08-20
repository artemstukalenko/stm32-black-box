#include "Sensor/Barometer/MockBarometer.h"
#include <stdio.h>
#include <stdlib.h>

MockBarometer::MockBarometer() : temperature_(25.0f), pressure_(101325.0f) {
	stringBuffer_[0] = '\0';
}

bool MockBarometer::init() {
	return true;
}

bool MockBarometer::update() {
	float tempDelta = ((float)(rand() % 10) - 5.0f) / 10.0f;
	float pressDelta = (float)(rand() % 100) - 50.0f;

	temperature_ += tempDelta;
	pressure_ += pressDelta;

	snprintf(stringBuffer_, sizeof(stringBuffer_), "BMP280 -> P: %d Pa, T: %d C\r\n",
			(int) pressure_, (int) temperature_);

	return true;
}

const char* MockBarometer::getDataString() {
	return stringBuffer_;
}

const char* MockBarometer::getName() {
	return "MockBarometer";
}

uint32_t MockBarometer::getDelay() {
	return (uint32_t) 2000;
}
