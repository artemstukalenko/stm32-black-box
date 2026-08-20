#ifndef BAROMETER_H
#define BAROMETER_H

#include <HardwareInterface/I2C/II2CBus.h>
#include "Sensor/ISensor.h"

typedef struct {
	float pressurePa;
	float temperatureC;
} BarometerReading;

class Barometer : public ISensor {

private:
	II2CBus* i2cBus_;
	uint16_t devAddress_;

	float temperature_;
	float pressure_;
	char stringBuffer_[64];
	bool consumed_;

	uint16_t dig_T1_;
	int16_t dig_T2_, dig_T3_;
	uint16_t dig_P1_;
	int16_t dig_P2_, dig_P3_, dig_P4_, dig_P5_, dig_P6_, dig_P7_, dig_P8_, dig_P9_;
	int32_t t_fine_;

	bool readCalibrationData();
	bool readRawData(int32_t * rawTemp, int32_t * rawPress);
	bool calculateCompensatedData(int32_t rawTemp, int32_t rawPress);

public:
	Barometer(II2CBus* i2cBus, uint16_t address = (0x76 << 1));
	~Barometer() override = default;

	bool init() override;
	bool update() override;
	const char* getDataString() override;
	const char* getName() override;
	uint32_t getDelay() override;
	BarometerReading getReading();
};

#endif
