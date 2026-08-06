#include <gtest/gtest.h>
#include "Service/MavLinkSender.h"

class MavLinkSenderTest : public ::testing::Test {

protected:
	MavLinkSender* sender;

	void SetUp() override {
		sender = new MavLinkSender();
	}

	void TearDown() override {
		delete sender;
	}

};

TEST_F(MavLinkSenderTest, PackedMessageDecodesBackToOriginalValues) {
	MavLinkSender sender;
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
