#include <gtest/gtest.h>
#include <cstring>
#include <cmath>

#include "Service/MavLinkPacketBuilder.h"
#include "Sensor/GPS/Gps.h"

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




class MavLinkPacketBuilderTest : public ::testing::Test {

protected:
	MavLinkPacketBuilder packetBuilder = MavLinkPacketBuilder(kSystemId, kComponentId);
	uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
};

TEST_F(MavLinkPacketBuilderTest, BarometerPackedMessageDecodesBackToOriginalValues) {
	BarometerReading reading {101325.0f, 22.5f};

	uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
	uint16_t len = packetBuilder.packBarometerReading(&reading, 1234, buffer);

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

TEST_F(MavLinkPacketBuilderTest, GpsValidFixEncodesAllFieldsCorrectly) {
	GpsReading reading = makeBaseGpsReading();
	uint32_t timeBootMs = 5000;

	uint16_t len = packetBuilder.packGpsReading(&reading, timeBootMs, buffer);
	ASSERT_GT(len, 0u);

	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_0);

	EXPECT_EQ(decoded.time_usec, (uint64_t) timeBootMs * 1000ULL);
	EXPECT_EQ(decoded.lat, (int32_t) std::llround(reading.latitude * 1e7));
	EXPECT_EQ(decoded.lon, (int32_t) std::llround(reading.longitude * 1e7));
	EXPECT_EQ(decoded.alt, (int32_t) std::llround(reading.altitudeMSL * 1000.0f));
	EXPECT_EQ(decoded.satellites_visible, reading.satellitesUsed);
}

TEST_F(MavLinkPacketBuilderTest, GpsNoFixZeroesPositionButPacketIsStillValid) {
	GpsReading reading = makeBaseGpsReading();
	reading.fixType = GpsFixType::NoFix;

	reading.latitude = 999.0f;
	reading.longitude = -999.0f;
	reading.altitudeMSL = -1234.0f;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_1);

	EXPECT_EQ(decoded.lat, 0);
	EXPECT_EQ(decoded.lon, 0);
	EXPECT_EQ(decoded.alt, 0);
}

TEST_F(MavLinkPacketBuilderTest, GpsNoGpsZeroesPositionButPacketIsStillValid) {
	GpsReading reading = makeBaseGpsReading();
	reading.fixType = GpsFixType::NoGps;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_2);

	EXPECT_EQ(decoded.lat, 0);
	EXPECT_EQ(decoded.lon, 0);
	EXPECT_EQ(decoded.alt, 0);
}

TEST_F(MavLinkPacketBuilderTest, GpsFix2DIsTreatedAsHasFix) {
	GpsReading reading = makeBaseGpsReading();
	reading.fixType = GpsFixType::Fix2D;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_3);

	EXPECT_EQ(decoded.lat, (int32_t) std::llround(reading.latitude * 1e7));
	EXPECT_EQ(decoded.lon, (int32_t) std::llround(reading.longitude * 1e7));
}

TEST_F(MavLinkPacketBuilderTest, GpsDgpsFixKeepsPositionData) {
	GpsReading reading = makeBaseGpsReading();
	reading.fixType = GpsFixType::DGPS;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_0);

	EXPECT_EQ(decoded.lat, (int32_t) std::llround(reading.latitude * 1e7));
	EXPECT_EQ(decoded.lon, (int32_t) std::llround(reading.longitude * 1e7));
}

TEST_F(MavLinkPacketBuilderTest, GpsHdopAtSentinelMapsToEphUnknown) {
	GpsReading reading = makeBaseGpsReading();
	reading.hdop = 99.99f;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_1);

	EXPECT_EQ(decoded.eph, UINT16_MAX);
}

TEST_F(MavLinkPacketBuilderTest, GpsNormalHdopIsScaledByHundred) {
	GpsReading reading = makeBaseGpsReading();
	reading.hdop = 1.5f;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_2);

	EXPECT_EQ(decoded.eph, 150);
}

TEST_F(MavLinkPacketBuilderTest, GpsEpvIsAlwaysUnknown) {
	GpsReading reading = makeBaseGpsReading();

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_3);

	EXPECT_EQ(decoded.epv, UINT16_MAX);
}

TEST_F(MavLinkPacketBuilderTest, GpsNegativeSpeedMapsToVelUnknown) {
	GpsReading reading = makeBaseGpsReading();
	reading.speedKnots = -1.0f;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_0);

	EXPECT_EQ(decoded.vel, UINT16_MAX);
}

TEST_F(MavLinkPacketBuilderTest, GpsValidSpeedConvertsFromKnotsToCmPerSec) {
	GpsReading reading = makeBaseGpsReading();
	reading.speedKnots=10.0f;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_1);

	uint16_t expected = (uint16_t) std::lround(10.0f * kKnotsToCmS);

	EXPECT_EQ(decoded.vel, expected);
}

TEST_F(MavLinkPacketBuilderTest, GpsSpeedAtZeroKnotsIsNotTreatedAsInvalid) {
	GpsReading reading = makeBaseGpsReading();
	reading.speedKnots=0.0f;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_2);

	EXPECT_EQ(decoded.vel, 0);
}

TEST_F(MavLinkPacketBuilderTest, GpsNegativeCourseMapsToUnknown) {
	GpsReading reading = makeBaseGpsReading();
	reading.courseDegrees=-1.0f;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_3);

	EXPECT_EQ(decoded.cog, UINT16_MAX);
}

TEST_F(MavLinkPacketBuilderTest, GpsValidCourseConvertsToCentidegrees) {
	GpsReading reading = makeBaseGpsReading();
	reading.courseDegrees=123.45f;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_0);

	uint16_t expected = (uint16_t) std::llround(123.45f * 100.0f);

	EXPECT_EQ(decoded.cog, expected);
}

TEST_F(MavLinkPacketBuilderTest, GpsCourseAtZeroDegreesIsNotTreatedAsInvalid) {
	GpsReading reading = makeBaseGpsReading();
	reading.courseDegrees=0.0f;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_1);

	EXPECT_EQ(decoded.cog, 0);
}

TEST_F(MavLinkPacketBuilderTest, GpsTimeUsecIsBootMsScaledToMicroseconds) {
	GpsReading reading = makeBaseGpsReading();
	uint32_t timeBootMs = 987654;

	uint16_t len = packetBuilder.packGpsReading(&reading, timeBootMs, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_2);

	EXPECT_EQ(decoded.time_usec, (uint64_t) timeBootMs * 1000ULL);
}

TEST_F(MavLinkPacketBuilderTest, GpsSatellitesUsedPassesThroughUnmodified) {
	GpsReading reading = makeBaseGpsReading();
	reading.satellitesUsed = 14;

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
	mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_3);

	EXPECT_EQ(decoded.satellites_visible, reading.satellitesUsed);
}

TEST_F(MavLinkPacketBuilderTest, FixTypeEnumMatchesMavlinkGpsFixTypeValues) {
	struct Case { GpsFixType fix; uint8_t expectedMavLinkValue; const char* name; };

	const Case cases[] = {
			{GpsFixType::NoGps, GPS_FIX_TYPE_NO_GPS, "NoGps"},
			{GpsFixType::NoFix, GPS_FIX_TYPE_NO_FIX, "NoFix"},
			{GpsFixType::Fix2D, GPS_FIX_TYPE_2D_FIX, "Fix2D"},
			{GpsFixType::Fix3D, GPS_FIX_TYPE_3D_FIX, "Fix3D"},
			{GpsFixType::DGPS, GPS_FIX_TYPE_DGPS, "DGPS"}
	};

	for (const auto& c : cases) {
		GpsReading reading = makeBaseGpsReading();
		reading.fixType = c.fix;

		uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);
		mavlink_gps_raw_int_t decoded = decodePackedGps(buffer, len, MAVLINK_COMM_0);

		EXPECT_EQ(decoded.fix_type, c.expectedMavLinkValue);
	}
}

TEST_F(MavLinkPacketBuilderTest, ReturnedLengthWithinMAvLinkV2Limits) {
	GpsReading reading = makeBaseGpsReading();

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);

	EXPECT_GT(len, 0u);
	EXPECT_LE(len, MAVLINK_MAX_PACKET_LEN);
}

TEST_F(MavLinkPacketBuilderTest, MessageIdSystemIdAndComponentIdAreCorrect) {
	GpsReading reading = makeBaseGpsReading();

	uint16_t len = packetBuilder.packGpsReading(&reading, 1000, buffer);

	mavlink_message_t msg {};
	mavlink_status_t status {};

	bool decoded = false;

	for (uint16_t i = 0; i < len; ++i) {
		if (mavlink_parse_char(MAVLINK_COMM_1, buffer[i], &msg, &status)) {
			decoded = true;
			break;
		}
	}

	ASSERT_TRUE(decoded);
	EXPECT_EQ(msg.msgid, static_cast<uint32_t>(MAVLINK_MSG_ID_GPS_RAW_INT));
	EXPECT_EQ(msg.sysid, kSystemId);
	EXPECT_EQ(msg.compid, kComponentId);
}
