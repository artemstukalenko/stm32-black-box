#include <gtest/gtest.h>
#include <cstring>
#include <cmath>

#include "Service/MavLinkSender.h"
#include "Sensor/GPS/GPS.h"

namespace {

constexpr uint8_t kSystemId = 1;
constexpr uint8_t kComponentId = 1;
constexpr float kKnotsToCmS = 51.4444f;

GpsReading makeBaseGpsReading() {
	GpsReading r {};
	r.fixType = GpsFixType::Fix3D;
	r.satellitesUsed = 8;
	r.hdop = 1.2f;
	r.latitude = 50.4501f;
	r.longitude = 30.5234f;
	r.altitudeMSL = 187.5f;
	r.speedKnots = 12.3f;
	r.courseDegrees = 245.6f;
	r.utcTimeOfFix = 123456;
	return r;
}

mavlink_gps_raw_int_t decodePackedGps(const uint8_t* buffer, uint16_t len, uint8_t channel) {
	mavlink_message_t msg{};
	mavlink_status_t status{};
	bool decoded = false;

	for (uint16_t i = 0; i < len; ++i) {
		if (mavlink_parse_char(channel, buffer[i], &msg, &status)) {
			decoded = true;
			break;
		}
	}

	EXPECT_TRUE(decoded);

	mavlink_gps_raw_int_t out{};
	mavlink_msg_gps_raw_int_decode(&msg, &out);
	return out;
}

}




class MavLinkSenderTest : public ::testing::Test {

protected:
	MavLinkSender sender = MavLinkSender(kSystemId, kComponentId);
	uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
};

TEST_F(MavLinkSenderTest, BarometerPackedMessageDecodesBackToOriginalValues) {
	BarometerReading reading {101325.0f, 22.5f};

	uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
	uint16_t len = sender.packBarometerReading(&reading, 1234, buffer);

	ASSERT_GT(len, 0);

	mavlink_message_t msg;
	mavlink_status_t status;
	bool messageParsed = false;

	for (uint16_t i = 0; i < len; i++) {
		if (mavlink_parse_char(MAVLINK_COMM_1, buffer[i], &msg, &status)) {
			messageParsed = true;
		}
	}

	ASSERT_TRUE(messageParsed);
	ASSERT_EQ(msg.msgid, MAVLINK_MSG_ID_SCALED_PRESSURE);

	mavlink_scaled_pressure_t decoded;
	mavlink_msg_scaled_pressure_decode(&msg, &decoded);

	EXPECT_NEAR(decoded.press_abs, 1013.25f, 0.01f);
	EXPECT_NEAR(decoded.temperature / 100.0f, 22.5f, 0.01f);
	EXPECT_EQ(decoded.time_boot_ms, 1234u);
}

TEST_F(MavLinkSenderTest, GpsValidFixEncodesAllFieldsCorrectly) {
	GpsReading reading = makeBaseGpsReading();
	uint32_t timeBootMs = 5000;

	uint16_t len = sender.packGpsReading(&reading, timeBootMs, buffer);
	ASSERT_GT(len, 0u);

	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_0);

	EXPECT_EQ(decoded.time_usec, (uint64_t) timeBootMs * 1000ULL);
	EXPECT_EQ(decoded.lat, (int32_t) std::llround(reading.latitude * 1e7));
	EXPECT_EQ(decoded.lon, (int32_t) std::llround(reading.longitude * 1e7));
	EXPECT_EQ(decoded.alt, (int32_t) std::llround(reading.altitudeMSL * 1000.0f));
	EXPECT_EQ(decoded.satellites_visible, reading.satellitesUsed);
}
