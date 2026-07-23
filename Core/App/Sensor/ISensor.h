#ifndef ISENSOR_H
#define ISENSOR_H

#include <stdint.h>

class ISensor {
public:
	virtual ~ISensor() = default;

	virtual bool init() = 0;

	virtual void update() = 0;

	virtual const char* getDataString() = 0;

	virtual const char* getName() = 0;

	virtual uint32_t getDelay() = 0;
};

#endif
