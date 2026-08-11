#include "GPS.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Gps::Gps(IUartBus* uartBus) : uartBus_(uartBus), rxByte_(0),
		sentenceIndex_(0) {
	queueHead_ = 0;
	queueTail_ = 0;
	droppedSentenceCount_ = 0;
	reading_ = {};
};


bool Gps::init() {
	return uartBus_->receive((uint8_t*) &rxByte_, 1);
}

bool Gps::update() {
	return true;
}

const char* Gps::getDataString() {
	if (queueTail_ == queueHead_) {
		popScratch_[0] = '\0';
		return popScratch_;
	}

	strncpy(popScratch_, sentenceQueue_[queueTail_], sizeof(popScratch_) - 1);
	popScratch_[sizeof(popScratch_) - 1] = '\0';
	queueTail_ = (queueTail_ + 1) % SENTENCE_QUEUE_CAPACITY;

	return popScratch_;
}

const char* Gps::getName() {
	return "NEO-6M GPS";
}

uint32_t Gps::getDelay() {
	return 1000;
}

void Gps::feedData(uint8_t byte) {
	if (byte == '\n' || byte == '\r') {
		if (sentenceIndex_ > 0) {
			sentenceBuffer_[sentenceIndex_] = '\0';

			if (sentenceBuffer_[0] == '$') {
				char formatted[MINMEA_MAX_SENTENCE_LENGTH];
				snprintf(formatted, sizeof(formatted), "%s\r\n", sentenceBuffer_);
				pushSentence(formatted);
				dispatchSentence(formatted);
			}

			sentenceIndex_ = 0;
		}
	} else {
		if (sentenceIndex_ < (sizeof(sentenceBuffer_) - 1)) {
			sentenceBuffer_[sentenceIndex_++] = (char)byte;
		} else {
			sentenceIndex_ = 0;
		}
	}
}

void Gps::pushSentence(const char* sentence) {
	uint8_t nextHead = (queueHead_ + 1) % SENTENCE_QUEUE_CAPACITY;

	if (nextHead == queueTail_) {
		droppedSentenceCount_++;
		return;
	}

	strncpy(sentenceQueue_[queueHead_], sentence, sizeof(sentenceQueue_[queueHead_]) - 1);
	sentenceQueue_[queueHead_][sizeof(sentenceQueue_[queueHead_]) - 1] = '\0';
	queueHead_ = nextHead;
}

void Gps::handleRxInterrupt() {
	feedData(rxByte_);

	uartBus_->receive((uint8_t*)&rxByte_, 1);
}

GpsReading Gps::getReading() {
	return reading_;
}

void Gps::dispatchSentence(const char* sentence) {
	switch(minmea_sentence_id(sentence, false)) {
		case MINMEA_SENTENCE_GGA: {
			struct minmea_sentence_gga frame;
			if (minmea_parse_gga(&frame, sentence)) {
				applyGga(frame);
			}
			break;
		}

		case MINMEA_SENTENCE_RMC: {
			struct minmea_sentence_rmc frame;
			if (minmea_parse_rmc(&frame, sentence)) {
				applyRmc(frame);
			}
			break;
		}

		default:
			break;
	}
}

void Gps::applyGga(const struct minmea_sentence_gga& frame) {
	switch (frame.fix_quality) {
		case 0: reading_.fixType = GpsFixType::NoFix; break;
		case 2: reading_.fixType = GpsFixType::DGPS; break;
		default: reading_.fixType = GpsFixType::Fix3D; break;
	}

	reading_.satellitesUsed = (uint8_t) frame.satellites_tracked;

	reading_.hdop = minmea_tofloat(&frame.hdop);
	if (frame.hdop.scale == 0) {
		reading_.hdop = 99.99f;
	}

	reading_.altitudeMSL = minmea_tofloat(&frame.altitude);

	if (reading_.fixType != GpsFixType::NoFix) {
		reading_.latitude = minmea_tocoord(&frame.latitude);
		reading_.longitude = minmea_tocoord(&frame.longitude);
	}

	reading_.utcTimeOfFix = (uint32_t) frame.time.hours * 10000
			+ (uint32_t) frame.time.minutes * 100
			+ (uint32_t) frame.time.seconds;
}

void Gps::applyRmc(const struct minmea_sentence_rmc& frame) {
	reading_.utcTimeOfFix = (uint32_t) frame.time.hours * 10000
				+ (uint32_t) frame.time.minutes * 100
				+ (uint32_t) frame.time.seconds;

	if (!frame.valid) {
		reading_.fixType = GpsFixType::NoFix;
		reading_.speedKnots = -1.0f;
		reading_.courseDegrees = -1.0f;
		return;
	}

	if (reading_.fixType == GpsFixType::NoGps || reading_.fixType == GpsFixType::NoFix) {
		reading_.fixType = GpsFixType::Fix3D;
	}

	reading_.latitude = minmea_tocoord(&frame.latitude);
	reading_.longitude = minmea_tocoord(&frame.longitude);

	reading_.speedKnots = minmea_tofloat(&frame.speed);
	reading_.courseDegrees = minmea_tofloat(&frame.course);
}
