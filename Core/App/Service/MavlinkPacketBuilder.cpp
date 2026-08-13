#include <Service/MavlinkPacketBuilder.h>
#include <cmath>

static constexpr float KNOTS_TO_CM_S = 51.4444f;

MavLinkPacketBuilder::MavLinkPacketBuilder(uint8_t systemId, uint8_t componentId) : systemId_(systemId), componentId_(componentId) {}

uint16_t MavLinkPacketBuilder::packBarometerReading(const BarometerReading* reading, uint32_t timeBootMs, uint8_t* buffer) {
	mavlink_message_t msg;

	mavlink_msg_scaled_pressure_pack(
			systemId_, componentId_,
			&msg,
			timeBootMs,
			reading->pressurePa / 100.0f,
			0.0f,
			(int16_t) (reading->temperatureC * 100.f),
			0
	);

	return mavlink_msg_to_send_buffer(buffer, &msg);
}

uint16_t MavLinkPacketBuilder::packGpsReading(const GpsReading* reading, uint32_t timeBootMs, uint8_t* buffer) {
	mavlink_message_t msg;

	uint8_t fixType = static_cast<uint8_t>(reading->fixType);
	bool hasFix = (reading->fixType == GpsFixType::Fix2D ||
			reading->fixType == GpsFixType::Fix3D ||
			reading->fixType == GpsFixType::DGPS);

	int32_t lat = hasFix ? (int32_t) std::llround(reading->latitude * 1e7) : 0;
	int32_t lon = hasFix ? (int32_t) std::llround(reading->longitude * 1e7) : 0;
	int32_t alt = hasFix ? (int32_t) std::llround(reading->altitudeMSL * 1000.0f) : 0;

	uint16_t eph = (reading->hdop >= 99.0f) ? UINT16_MAX
					: (uint16_t) std::llround(reading->hdop * 100.0f);
	uint16_t epv = UINT16_MAX;

	uint16_t vel = (reading->speedKnots < 0.0f) ? UINT16_MAX
			: (uint16_t) std::llround(reading->speedKnots * KNOTS_TO_CM_S);

	uint16_t cog = (reading->courseDegrees < 0.0f) ? UINT16_MAX
			: (uint16_t) std::llround(reading->courseDegrees * 100.0f);

	uint64_t timeUsec = (uint64_t) timeBootMs * 1000ULL;

	uint8_t satellitesUsed = reading->satellitesUsed;

	mavlink_msg_gps_raw_int_pack(
			systemId_, componentId_,
			&msg, timeUsec, fixType,
			lat, lon, alt, eph, epv,
			vel, cog, satellitesUsed,
			0, 0, 0, 0, 0, 0
	);

	return mavlink_msg_to_send_buffer(buffer, &msg);
}

uint16_t MavLinkPacketBuilder::packHeartbeat(uint8_t* buffer) {
	mavlink_message_t msg;

	mavlink_msg_heartbeat_pack(
			systemId_, componentId_,
			&msg, MAV_TYPE_GENERIC,
			MAV_AUTOPILOT_GENERIC,
			0, 0,
			MAV_STATE_ACTIVE
	);

	return mavlink_msg_to_send_buffer(buffer, &msg);
}
