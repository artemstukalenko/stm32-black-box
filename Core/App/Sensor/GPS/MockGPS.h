#ifndef MOCKGPS_H
#define MOCKGPS_H

#include "Sensor/ISensor.h"

class MockGPS : public ISensor {

private:
	double latitude;
	double longitude;
	float altitude;
	char stringBuffer[64];

public:
	MockGPS();

	~MockGPS() override = default;

	bool init() override;
	void update() override;
	const char* getDataString() override;
	const char* getName() override;
	uint32_t getDelay() override;
};

#endif
