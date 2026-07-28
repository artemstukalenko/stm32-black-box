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
