#ifndef MOCKBAROMETER_H
#define MOCKBAROMETER_H

#include "Sensor/ISensor.h"

class MockBarometer : public ISensor {

private:
	float temperature;
	float pressure;
	char stringBuffer[64];

public:
	MockBarometer();

	~MockBarometer() override = default;

	bool init() override;
	void update() override;
	const char* getDataString() override;
	const char* getName() override;
};

#endif
