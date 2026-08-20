#include "Sensor/Barometer/Barometer.h"
#include <stdio.h>
#include <string.h>

#define BMP280_REG_ID 0xD0
#define BMP280_REG_CTRL_MEAS 0xF4
#define BMP280_REG_CONFIG 0xF5
#define BMP280_REG_PRESS_MSB 0xF7
#define BMP280_REG_CALIB 0x88

Barometer::Barometer(II2CBus* i2cBus, uint16_t address) : i2cBus_(i2cBus), devAddress_(address),
		temperature_(0.0f), pressure_(0.0f) {
	stringBuffer_[0] = '\0';
}

bool Barometer::init() {
	uint8_t chipId = 0;

	//read chipId
	if (!i2cBus_->memRead(devAddress_, BMP280_REG_ID, 1, &chipId, 1, 1000)) {
		return false;
	}

	if (chipId != 0x58) {
		return false; //i2c1 is not BMP280
	}

	if (!readCalibrationData()) {
		return false;
	}

	uint8_t ctrl_meas = (1 << 5) | (1 << 2) | 3;
	i2cBus_->memWrite(devAddress_, BMP280_REG_CTRL_MEAS, 1, &ctrl_meas, 1, 1000);

	uint8_t config = (5 << 5) | (0 << 2);
	i2cBus_->memWrite(devAddress_, BMP280_REG_CONFIG, 1, &config, 1, 1000);

	return true;
}

bool Barometer::update() {
	bool updateResult = true;

	int32_t rawTemp = 0, rawPress = 0;

	updateResult &= readRawData(&rawTemp, &rawPress);

	//transform to meaningful temperature and pressure (C and Pa)
	updateResult &= calculateCompensatedData(rawTemp, rawPress);

	snprintf(stringBuffer_, sizeof(stringBuffer_), "P: %d Pa, T: %d C\r\n",
				(int) pressure_, (int) temperature_);

	consumed_ = false;

	updateResult &= strlen(stringBuffer_) > 0;

	return updateResult;
}

const char* Barometer::getDataString() {
	if (consumed_) {
		return "";
	} else {
		consumed_ = true;
		return stringBuffer_;
	}
}

const char* Barometer::getName() {
	return "BMP280";
}

uint32_t Barometer::getDelay() {
	return 2000;
}

BarometerReading Barometer::getReading() {
	return BarometerReading{pressure_, temperature_};
}

bool Barometer::readCalibrationData() {
	uint8_t calibData[24];

	if (!i2cBus_->memRead(devAddress_, BMP280_REG_CALIB, 1, calibData, 24, 1000)) {
		return false;
	}

	dig_T1_ = (calibData[1] << 8) | calibData[0];
	dig_T2_ = (calibData[3] << 8) | calibData[2];
	dig_T3_ = (calibData[5] << 8) | calibData[4];
	dig_P1_ = (calibData[7] << 8) | calibData[6];
	dig_P2_ = (calibData[9] << 8) | calibData[8];
	dig_P3_ = (calibData[11] << 8) | calibData[10];
	dig_P4_ = (calibData[13] << 8) | calibData[12];
	dig_P5_ = (calibData[15] << 8) | calibData[14];
	dig_P6_ = (calibData[17] << 8) | calibData[16];
	dig_P7_ = (calibData[19] << 8) | calibData[18];
	dig_P8_ = (calibData[21] << 8) | calibData[20];
	dig_P9_ = (calibData[23] << 8) | calibData[22];

	return true;
}

bool Barometer::readRawData(int32_t* rawTemp, int32_t* rawPress) {
	uint8_t data[6];

	if (i2cBus_->memRead(devAddress_, BMP280_REG_PRESS_MSB, 1, data, 6, 1000)) {
		*rawPress = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
		*rawTemp  = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
		return true;
	} else {
		return false;
	}
}

bool Barometer::calculateCompensatedData(int32_t rawTemp, int32_t rawPress) {
	// Compensation formulas from Bosch BMP280 Datasheet

	//Temperature compensation
	int32_t var1, var2;
	var1 = ((((rawTemp >> 3) - ((int32_t)dig_T1_ << 1))) * ((int32_t)dig_T2_)) >> 11;
	var2 = (((((rawTemp >> 4) - ((int32_t)dig_T1_)) * ((rawTemp >> 4) - ((int32_t)dig_T1_))) >> 12) * ((int32_t)dig_T3_)) >> 14;
	t_fine_ = var1 + var2;

	float T = (t_fine_ * 5 + 128) >> 8;
	temperature_ = T / 100.0f; // Convert to C

	//Pressure compensation
	int64_t p_var1, p_var2, p;
	p_var1 = ((int64_t)t_fine_) - 128000;
	p_var2 = p_var1 * p_var1 * (int64_t)dig_P6_;
	p_var2 = p_var2 + ((p_var1 * (int64_t)dig_P5_) << 17);
	p_var2 = p_var2 + (((int64_t)dig_P4_) << 35);
	p_var1 = ((p_var1 * p_var1 * (int64_t)dig_P3_) >> 8) + ((p_var1 * (int64_t)dig_P2_) << 12);
	p_var1 = (((((int64_t)1) << 47) + p_var1)) * ((int64_t)dig_P1_) >> 33;

	if (p_var1 == 0) {
		pressure_ = 0.0f;
		return false;
	}

	p = 1048576 - rawPress;
	p = (((p << 31) - p_var2) * 3125) / p_var1;
	p_var1 = (((int64_t)dig_P9_) * (p >> 13) * (p >> 13)) >> 25;
	p_var2 = (((int64_t)dig_P8_) * p) >> 19;

	p = ((p + p_var1 + p_var2) >> 8) + (((int64_t)dig_P7_) << 4);

	pressure_ = (float)p / 256.0f; //Convert to Pa

	return true;
}
