#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Sensor/Barometer/Barometer.h"
#include "MockHardwareInterface/I2C/MockI2CBus.h"

using ::testing::_;
using ::testing::Return;
using ::testing::SetArrayArgument;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Pointee;

class BarometerTest : public ::testing::Test {
protected:
	MockI2CBus i2c;
	Barometer* barometer;

	const uint16_t testAddress = 0x76 << 1;

	void SetUp() override {
		barometer = new Barometer(&i2c);
	}

	void TearDown() override {
		delete barometer;
	}
};

TEST_F(BarometerTest, InitSuccess) {
	uint8_t expectedChipId = 0x58;

	EXPECT_CALL(i2c, memRead(testAddress, 0xD0, 1, _, 1, _))
		.WillOnce(DoAll(SetArrayArgument<3>(&expectedChipId, &expectedChipId + 1), Return(true)));

	EXPECT_CALL(i2c, memRead(testAddress, 0x88, 1, _, _, _))
		.WillOnce(Return(true));

	EXPECT_TRUE(barometer->init());
}

TEST_F(BarometerTest, InitFailsWhenI2CReadFails) {
	EXPECT_CALL(i2c, memRead(testAddress, 0xD0, 1, _, 1, _))
			.WillOnce(Return(false));

	EXPECT_FALSE(barometer->init());
}

TEST_F(BarometerTest, InitFailsWhenCalibrationReadFails) {
	uint8_t expectedChipId = 0x58;

	EXPECT_CALL(i2c, memRead(testAddress, 0xD0, 1, _, 1, _))
		.WillOnce(DoAll(SetArrayArgument<3>(&expectedChipId, &expectedChipId + 1), Return(true)));

	EXPECT_CALL(i2c, memRead(testAddress, 0x88, 1, _, _, _))
		.WillOnce(Return(false));

	EXPECT_FALSE(barometer->init());
}

TEST_F(BarometerTest, InitWritesConfigurationRegisters) {
	uint8_t expectedChipId = 0x58;

	EXPECT_CALL(i2c, memRead(testAddress, 0xD0, 1, _, 1, _))
		.WillOnce(DoAll(SetArrayArgument<3>(&expectedChipId, &expectedChipId + 1), Return(true)));

	EXPECT_CALL(i2c, memRead(testAddress, 0x88, 1, _, _, _))
	        .WillOnce(Return(true));

	uint8_t expectedCtrlMeas = (1 << 5) | (1 << 2) | 3;
	uint8_t expectedConfig = (5 << 5) | (0 << 2);

	EXPECT_CALL(i2c, memWrite(
			testAddress,
			0xF4,
			1,
			Pointee(expectedCtrlMeas),
			1,
			1000
	)).WillOnce(Return(true));

	EXPECT_CALL(i2c, memWrite(
				testAddress,
				0xF5,
				1,
				Pointee(expectedConfig),
				1,
				1000
		)).WillOnce(Return(true));

	EXPECT_TRUE(barometer->init());
}
