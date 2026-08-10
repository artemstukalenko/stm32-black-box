#ifndef GPS_H
#define GPS_H

#include "Sensor/ISensor.h"
#include "HardwareInterface/UART/IUartBus.h"
#ifdef UNIT_TESTING
#include <gtest/gtest_prod.h>
#endif

typedef struct {
	double latitude;
	double longitude;
	uint32_t utcTimeOfFix;
	bool fixValid;
} GpsReading;

class Gps : public ISensor {

private:

#ifdef UNIT_TESTING
	FRIEND_TEST(GpsTest, FeedData_ParsesValidNmeaSentence_WithLineFeed);
	FRIEND_TEST(GpsTest, FeedData_ParsesValidNmeaSentence_WithCarriageReturn);
	FRIEND_TEST(GpsTest, FeedData_IgnoresInvalidSentences);
	FRIEND_TEST(GpsTest, FeedData_PreventsBufferOverflow);
#endif

	static constexpr uint8_t SENTENCE_QUEUE_CAPACITY = 16;
	char sentenceQueue_[SENTENCE_QUEUE_CAPACITY][128];
	volatile uint8_t queueHead_;
	volatile uint8_t queueTail_;
	uint32_t droppedSentenceCount_;
	char popScratch_[128];

	IUartBus* uartBus_;
	char rxByte_;
	uint8_t sentenceIndex_;
	char sentenceBuffer_[128];
	GpsReading reading_;

	void feedData(uint8_t byte);
	void pushSentence(const char* sentence);
	bool parseGpgll(const char* sentence, GpsReading* out);

public:
	explicit Gps(IUartBus* uartBus_);
	~Gps() override = default;

	bool init() override;
	bool update() override;
	const char* getDataString() override;
	const char* getName() override;
	uint32_t getDelay() override;

	void handleRxInterrupt();

	GpsReading getReading();

};

#endif
