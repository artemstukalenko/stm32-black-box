#ifndef MOCKBAROMETER_H
#define MOCKBAROMETER_H

#include "Sensor/ISensor.h"

class MockBarometer : public ISensor {

private:
	float temperature_;
	float pressure_;
	char stringBuffer_[64];

public:
	MockBarometer();

	~MockBarometer() override = default;

	bool init() override;
	bool update() override;
	const char* getDataString() override;
	const char* getName() override;
	uint32_t getDelay() override;
};

#endif
