#include "GPS.h"
#include <stdio.h>
#include <string.h>

Gps::Gps(UART_HandleTypeDef* huart) : huart_(huart), rxByte_(0),
		sentenceIndex_(0), newDataAvailable_(false) {
	sentenceBuffer[0] = '\0';
	stringBuffer[0] = '\0';
};


bool Gps::init() {
	HAL_StatusTypeDef status = HAL_UART_Receive_IT(huart_, (uint8_t*) &rxByte_, 1);
	return (status == HAL_OK);
}

void Gps::update() {
	//do nothing - stringBuffer is accumulated within feedData()
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

	HAL_UART_Receive_IT(huart_, (uint8_t*)&rxByte_, 1);
}
