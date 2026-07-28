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

TEST_F(BarometerTest, InitFailsWhenI2CErrorOnChipId) {
	EXPECT_CALL(i2c, memRead(testAddress, 0xD0, 1, _, 1, _))
			.WillOnce(Return(false));

	EXPECT_FALSE(barometer->init());
}

TEST_F(BarometerTest, InitFailsWhenWrongChipIdRecieved) {
	uint8_t wrongChipId = 0x00;

	EXPECT_CALL(i2c, memRead(testAddress, 0xD0, 1, _, 1, _))
		.WillOnce(DoAll(SetArrayArgument<3>(&wrongChipId, &wrongChipId + 1), Return(true)));

	EXPECT_FALSE(barometer->init());
}

TEST_F(BarometerTest, UpdateFailsWhenI2CReadFails) {
	EXPECT_CALL(i2c, memRead(testAddress, 0xF7, 1, _, 6, _))
			.WillOnce(Return(false));

	EXPECT_FALSE(barometer->update());
}

TEST_F(BarometerTest, UpdateSuccessAndReadsData) {
	uint8_t expectedChipId = 0x58;
	uint8_t calibData[24];

	for(int i = 0; i < 24; i++) {
	        calibData[i] = 1;
	}

	EXPECT_CALL(i2c, memRead(testAddress, 0xD0, 1, _, 1, _))
		.WillOnce(DoAll(SetArrayArgument<3>(&expectedChipId, &expectedChipId + 1), Return(true)));

	EXPECT_CALL(i2c, memRead(testAddress, 0x88, 1, _, 24, _))
		.WillOnce(DoAll(SetArrayArgument<3>(calibData, calibData + 24), Return(true)));

	EXPECT_CALL(i2c, memWrite(testAddress, 0xF4, 1, _, 1, _)).WillOnce(Return(true));
	    EXPECT_CALL(i2c, memWrite(testAddress, 0xF5, 1, _, 1, _)).WillOnce(Return(true));

	ASSERT_TRUE(barometer->init());

	uint8_t dummyData[6] = {
	        0x50, 0x00, 0x00,
	        0x50, 0x00, 0x00
	    };

	EXPECT_CALL(i2c, memRead(testAddress, 0xF7, 1, _, 6, _))
		.WillOnce(DoAll(
				SetArrayArgument<3>(dummyData, dummyData + 6),
				Return(true)
			)
		);

	bool result = barometer->update();

	EXPECT_TRUE(result);
}

TEST_F(BarometerTest, GetNameReturnsCorrectName) {
	EXPECT_STREQ(barometer->getName(), "BMP280");
}

TEST_F(BarometerTest, DataStringIsNotEmptyAfterUpdate) {
	uint8_t expectedChipId = 0x58;
		uint8_t calibData[24];

		for(int i = 0; i < 24; i++) {
		        calibData[i] = 1;
		}

		EXPECT_CALL(i2c, memRead(testAddress, 0xD0, 1, _, 1, _))
			.WillOnce(DoAll(SetArrayArgument<3>(&expectedChipId, &expectedChipId + 1), Return(true)));

		EXPECT_CALL(i2c, memRead(testAddress, 0x88, 1, _, 24, _))
			.WillOnce(DoAll(SetArrayArgument<3>(calibData, calibData + 24), Return(true)));

		EXPECT_CALL(i2c, memWrite(testAddress, 0xF4, 1, _, 1, _)).WillOnce(Return(true));
		    EXPECT_CALL(i2c, memWrite(testAddress, 0xF5, 1, _, 1, _)).WillOnce(Return(true));

		ASSERT_TRUE(barometer->init());

		uint8_t dummyData[6] = {
		        0x50, 0x00, 0x00,
		        0x50, 0x00, 0x00
		    };

		EXPECT_CALL(i2c, memRead(testAddress, 0xF7, 1, _, 6, _))
			.WillOnce(DoAll(
					SetArrayArgument<3>(dummyData, dummyData + 6),
					Return(true)
				)
			);

		barometer->update();

		const char* resultString = barometer->getDataString();

		ASSERT_NE(resultString, nullptr);
		EXPECT_GT(strlen(resultString), 0);
}
