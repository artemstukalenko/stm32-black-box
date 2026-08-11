#ifndef GPS_H
#define GPS_H

#include "Sensor/ISensor.h"
#include "HardwareInterface/UART/IUartBus.h"
#include "minmea.h"
#ifdef UNIT_TESTING
#include <gtest/gtest_prod.h>
#endif

enum class GpsFixType : uint8_t {
	NoGps = 0,
	NoFix = 1,
	Fix2D = 2,
	Fix3D = 3,
	DGPS = 4
};

typedef struct {
	double latitude;
	double longitude;
	float altitudeMSL;
	float hdop;
	float speedKnots;
	float courseDegrees;
	uint8_t satellitesUsed;
	uint32_t utcTimeOfFix;
	GpsFixType fixType;
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
	char sentenceQueue_[SENTENCE_QUEUE_CAPACITY][MINMEA_MAX_SENTENCE_LENGTH];
	volatile uint8_t queueHead_;
	volatile uint8_t queueTail_;
	uint32_t droppedSentenceCount_;
	char popScratch_[MINMEA_MAX_SENTENCE_LENGTH];

	IUartBus* uartBus_;
	char rxByte_;
	uint8_t sentenceIndex_;
	char sentenceBuffer_[MINMEA_MAX_SENTENCE_LENGTH];
	GpsReading reading_;

	void feedData(uint8_t byte);
	void pushSentence(const char* sentence);
	void dispatchSentence(const char* sentence);
	void applyGga(const struct minmea_sentence_gga& frame);
	void applyRmc(const struct minmea_sentence_rmc& frame);

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
