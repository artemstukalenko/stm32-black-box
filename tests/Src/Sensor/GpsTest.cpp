#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string.h>
#include "Sensor/GPS/Gps.h"
#include "MockHardwareInterface/UART/MockUartBus.h"

using ::testing::_;
using ::testing::Return;
using ::testing::SetArrayArgument;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Pointee;

class GpsTest : public ::testing::Test {

protected:
	MockUartBus uart;
	Gps* gps;

	void SetUp() override {
		gps = new Gps(&uart);
	}

	void TearDown() override {
		delete gps;
	}

	void feedSentence(const char * sentence) {
		for (const char* p = sentence; *p != '\0'; ++p) {
			gps->feedData((uint8_t) *p);
		}

		gps->feedData((uint8_t) '\n');
	}
};

TEST_F(GpsTest, InitSuccess) {
	EXPECT_CALL(uart, receive(_, 1))
			.WillOnce(Return(true));

	EXPECT_TRUE(gps->init());
}

TEST_F(GpsTest, InitFailure) {
	EXPECT_CALL(uart, receive(_, 1))
				.WillOnce(Return(false));

	EXPECT_FALSE(gps->init());
}

TEST_F(GpsTest, UpdateAlwaysReturnsTrue) {
	EXPECT_TRUE(gps->update()); //stub method
}

TEST_F(GpsTest, GetNameReturnsCorrectName) {
	EXPECT_STREQ(gps->getName(), "NEO-6M GPS");
}

TEST_F(GpsTest, FeedData_ParsesValidNmeaSentence_WithLineFeed) {
	const char* sentence = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A";

	for (int i = 0; sentence[i] != '\0'; ++i) {
		gps->feedData(sentence[i]);
	}

	EXPECT_STREQ(gps->getDataString(), "");

	gps->feedData((uint8_t)'\n');

	std::string expected = std::string(sentence) + "\r\n";

	EXPECT_STREQ(gps->getDataString(), expected.c_str());
}

TEST_F(GpsTest, FeedData_ParsesValidNmeaSentence_WithCarriageReturn) {
	const char* sentence = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47";

	for (int i = 0; sentence[i] != '\0'; ++i) {
		gps->feedData(sentence[i]);
	}

	gps->feedData('\r');

	std::string expected = std::string(sentence) + "\r\n";
	EXPECT_STREQ(gps->getDataString(), expected.c_str());
}

TEST_F(GpsTest, FeedData_IgnoresInvalidSentences) {
	const char* invalidSentence = "NOT,A,VALID,NMEA,SENTENCE,WITHOUT,DOLLAR";

	for (int i = 0; invalidSentence[i] != '\0'; ++i) {
		gps->feedData(invalidSentence[i]);
	}

	gps->feedData('\n');

	EXPECT_STREQ(gps->getDataString(), "");
}

TEST_F(GpsTest, FeedData_PreventsBufferOverflow) {
	gps->feedData('$');

	for (int i = 0; i < 256; ++i) {
		gps->feedData('A');
	}

	gps->feedData('\n');

	std::string result = gps->getDataString();

	EXPECT_EQ(result[0], '\0');
}

TEST_F(GpsTest, FeedData_ParsesGdaWithFix_UpdatesReadingFields) {
	feedSentence("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47");

	GpsReading reading = gps->getReading();

	EXPECT_EQ(reading.fixType, GpsFixType::Fix3D);
	EXPECT_EQ(reading.satellitesUsed, 8);
	EXPECT_NEAR(reading.hdop, 0.9f, 1e-3f);
	EXPECT_NEAR(reading.altitudeMSL, 545.4f, 1e-2f);
	EXPECT_NEAR(reading.latitude, 48.11730, 1e-4);
	EXPECT_NEAR(reading.longitude, 11.516667, 1e-4);
	EXPECT_EQ(reading.utcTimeOfFix, 123519u);
}

TEST_F(GpsTest, FeedData_ParsesGgaNoFix_DoesNotTouchPosition) {
	// fix_quality = 0, latitude/longitude are blank
	feedSentence("$GPGGA,123519,,,,,0,00,,,,,,,*6B");

	GpsReading reading = gps->getReading();

	EXPECT_EQ(reading.fixType, GpsFixType::NoFix);
	EXPECT_EQ(reading.satellitesUsed, 0);
	EXPECT_DOUBLE_EQ(reading.latitude, 0.0);
	EXPECT_DOUBLE_EQ(reading.longitude, 0.0);
}

TEST_F(GpsTest, FeedData_ParsesGgaWithFixQuality2_SetsDgpsFixType) {
	feedSentence("$GPGGA,123519,4807.038,N,01131.000,E,2,08,0.9,545.4,M,46.9,M,,*44");

	EXPECT_EQ(gps->getReading().fixType, GpsFixType::DGPS);
}

TEST_F(GpsTest, FeedData_WithMissingHdopField_UsesSentinelValue) {
	feedSentence("$GPGGA,123519,4807.038,N,01131.000,E,1,08,,545.4,M,46.9,M,,*60");

	EXPECT_EQ(gps->getReading().hdop, 99.99f);
}

TEST_F(GpsTest, FeedData_GgaWithExplicitZeroHdop_KeepsRealZeroNotSentinel) {
	feedSentence("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.0,545.4,M,46.9,M,,*4E");

	EXPECT_EQ(gps->getReading().hdop, 0.0f);
}

TEST_F(GpsTest, FeedData_ParsesRmcValid_UpdatesSpeedCourseAndPosition) {
	feedSentence("$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A");

	GpsReading reading = gps->getReading();

	EXPECT_EQ(reading.fixType, GpsFixType::Fix3D);
	EXPECT_NEAR(reading.latitude, 48.11730, 1e-4);
	EXPECT_NEAR(reading.longitude, 11.516667, 1e-4);
	EXPECT_NEAR(reading.speedKnots, 22.4f, 1e-2f);
	EXPECT_NEAR(reading.courseDegrees, 84.4f, 1e-2f);
	EXPECT_EQ(reading.utcTimeOfFix, 123519u);
}

TEST_F(GpsTest, FeedData_ParsesRmcInvalid_SetsNoFixAndNegativeSpeedCourse) {
	feedSentence("$GPRMC,123519,V,,,,,,,230394,,*33");

	GpsReading reading = gps->getReading();

	EXPECT_EQ(reading.fixType, GpsFixType::NoFix);
	EXPECT_FLOAT_EQ(reading.speedKnots, -1.0f);
	EXPECT_FLOAT_EQ(reading.courseDegrees, -1.0f);
	//must be updated even for invalid sentence
	EXPECT_EQ(reading.utcTimeOfFix, 123519u);
}

TEST_F(GpsTest, FeedData_RmcDoesNotDowngradeExistingDgpsFixType) {
	feedSentence("$GPGGA,123519,4807.038,N,01131.000,E,2,08,0.9,545.4,M,46.9,M,,*44");
	ASSERT_EQ(gps->getReading().fixType, GpsFixType::DGPS);

	feedSentence("$GPRMC,123520,A,4807.038,N,01131.000,E,010.0,090.0,230394,,*1F");

	EXPECT_EQ(gps->getReading().fixType, GpsFixType::DGPS);
}

TEST_F(GpsTest, FeedData_UnhandledSentenceType_DoesNotModifyReading) {
	feedSentence("$GPGLL,4916.45,N,12311.12,W,225444,A*31");

	GpsReading reading = gps->getReading();

	EXPECT_EQ(reading.fixType, GpsFixType::NoGps);
	EXPECT_EQ(reading.utcTimeOfFix, 0u);
	EXPECT_DOUBLE_EQ(reading.latitude, 0.0);
	EXPECT_DOUBLE_EQ(reading.longitude, 0.0);
}

TEST_F(GpsTest, FeedData_MultipleSentencesAccumulateIntoLatestReading) {
	feedSentence("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47");
	feedSentence("$GPRMC,123520,A,4807.038,N,01131.000,E,010.0,090.0,230394,,*1F");

	GpsReading reading = gps->getReading();

	EXPECT_EQ(reading.fixType, GpsFixType::Fix3D);
	EXPECT_EQ(reading.satellitesUsed, 8);
	EXPECT_NEAR(reading.altitudeMSL, 545.4f, 1e-2f);
	EXPECT_NEAR(reading.speedKnots, 10.0f, 1e-2f);
	EXPECT_NEAR(reading.courseDegrees, 90.0f, 1e-2f);
	EXPECT_EQ(reading.utcTimeOfFix, 123520u);
}
