#ifndef MAVLINK_PACKET_BUILDER_H
#define MAVLINK_PACKET_BUILDER_H

#include <cstdint>
#include "common/mavlink.h"
#include "Sensor/Barometer/Barometer.h"
#include "Sensor/GPS/Gps.h"

class MavLinkPacketBuilder {

private:
	uint8_t systemId_;
	uint8_t componentId_;

public:
	MavLinkPacketBuilder(uint8_t systemId = 1, uint8_t componentId = 191);

	uint16_t packBarometerReading(const BarometerReading* reading, uint32_t timeBootMs, uint8_t* buffer);

	uint16_t packGpsReading(const GpsReading* reading,  uint32_t timeBootMs, uint8_t* buffer);

};

#endif
