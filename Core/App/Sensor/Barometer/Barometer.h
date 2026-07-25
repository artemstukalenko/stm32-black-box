#ifndef BAROMETER_H
#define BAROMETER_H

#include "Sensor/ISensor.h"
#include "i2c.h"

class Barometer : public ISensor {

private:
	I2C_HandleTypeDef* hi2c_;
	uint16_t devAddress_;

	float temperature_;
	float pressure_;
	char stringBuffer_[64];

	uint16_t dig_T1;
	int16_t dig_T2, dig_T3;
	uint16_t dig_P1;
	int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
	int32_t t_fine;

	bool readCalibrationData();
	void readRawData(int32_t * rawTemp, int32_t * rawPress);
	void calculateCompensatedData(int32_t rawTemp, int32_t rawPress);

public:
	Barometer(I2C_HandleTypeDef* hi2c, uint16_t address = (0x76 << 1));
	~Barometer() override = default;

	bool init() override;
	void update() override;
	const char* getDataString() override;
	const char* getName() override;
	uint32_t getDelay() override;
};

#endif
