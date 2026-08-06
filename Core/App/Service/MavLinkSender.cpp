#include "MavLinkSender.h"

MavLinkSender::MavLinkSender(uint8_t systemId, uint8_t componentId) : systemId_(systemId), componentId_(componentId) {}

uint16_t MavLinkSender::packBarometerReading(const BarometerReading* reading, uint32_t timeBootMs, uint8_t* buffer) {
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
