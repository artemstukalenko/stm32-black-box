#include "GPS.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Gps::Gps(IUartBus* uartBus) : uartBus_(uartBus), rxByte_(0),
		sentenceIndex_(0), newDataAvailable_(false) {
	sentenceBuffer[0] = '\0';
	stringBuffer[0] = '\0';
	reading_ = {};
};


bool Gps::init() {
	return uartBus_->receive((uint8_t*) &rxByte_, 1);
}

bool Gps::update() {
	//do nothing - stringBuffer is accumulated within feedData()
	return true;
}

const char* Gps::getDataString() {
	return stringBuffer;
}

const char* Gps::getName() {
	return "NEO-6M GPS";
}

uint32_t Gps::getDelay() {
	return 5000;
}

void Gps::feedData(uint8_t byte) {
	if (byte == '\n' || byte == '\r') {
		if (sentenceIndex_ > 0) {
			sentenceBuffer[sentenceIndex_] = '\0';

			if (sentenceBuffer[0] == '$') {
				snprintf(stringBuffer, sizeof(stringBuffer), "%s\r\n", sentenceBuffer);
				newDataAvailable_ = true;
				parseGpgll(stringBuffer, &reading_);
			}

			sentenceIndex_ = 0;
		}
	} else {
		if (sentenceIndex_ < (sizeof(sentenceBuffer) - 1)) {
			sentenceBuffer[sentenceIndex_++] = (char)byte;
		} else {
			sentenceIndex_ = 0;
		}
	}
}

void Gps::handleRxInterrupt() {
	feedData(rxByte_);

	uartBus_->receive((uint8_t*)&rxByte_, 1);
}

GpsReading Gps::getReading() {
	return reading_;
}

static double nmmToDecimalDegrees(const char* nmmStr) {
	if (nmmStr == NULL || strlen(nmmStr) == 0) {
		return 0.0;
	}

	double raw = atof(nmmStr);
	double degrees = (double)((int)(raw / 100.0));
	int wholePart = (int) raw;
	int deg = wholePart / 100;
	double minutes = raw - (deg * 100.0);

	return (double) deg + minutes / 60.0;
}

static bool verifyNmeaChecksum(const char* sentence) {
	const char* start = strchr(sentence, '$');
	const char* asterisk = strchr(sentence, '*');

	if (start == NULL || asterisk == NULL || asterisk <= start) {
		return false;
	}

	uint8_t checksum = 0;

	for (const char* p = start + 1; p < asterisk; ++p) {
		checksum ^= (uint8_t) (*p);
	}

	uint8_t expected = (uint8_t) strtol(asterisk + 1, NULL, 16);
	return checksum == expected;
}

static uint32_t parseUtcTime(const char* utcStr) {
	if (utcStr == NULL || strlen(utcStr) < 6) {
		return 0;
	}

	char buf[7] = {0};
	memcpy(buf, utcStr, 6);
	return (uint32_t) atol(buf);
}

bool Gps::parseGpgll(const char* sentence, GpsReading* out) {
	if (sentence == NULL || out == NULL) {
		return false;
	}

	if (!verifyNmeaChecksum(sentence)) {
		return false;
	}

	char buf[128];
	strncpy(buf, sentence, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';

	char* saveptr = NULL;
	char* token;

	token = strtok_r(buf, ",", &saveptr);

	if (token == NULL || strstr(token, "GLL") == NULL) {
		return false;
	}

	char* latStr = strtok_r(NULL, ",", &saveptr);
	char* latHemi = strtok_r(NULL, ",", &saveptr);
	char* lonStr = strtok_r(NULL, ",", &saveptr);
	char* lonHemi = strtok_r(NULL, ",", &saveptr);
	char* utcStr = strtok_r(NULL, ",", &saveptr);
	char* statusStr = strtok_r(NULL, ",", &saveptr);

	if (latStr == NULL || latHemi == NULL || lonStr == NULL || lonHemi == NULL || utcStr == NULL || statusStr == NULL) {
		return false;
	}

	bool valid = (statusStr[0] == 'A');

	double lat = nmmToDecimalDegrees(latStr);
	if (latHemi[0] == 'S') {
		lat = -lat;
	}

	double lon = nmmToDecimalDegrees(lonStr);
	if (lonHemi[0] == 'W') {
		lon = -lon;
	}

	out->latitude = lat;
	out->longitude = lon;
	out->utcTimeOfFix = parseUtcTime(utcStr);
	out->fixValid = valid;

	return true;
}
