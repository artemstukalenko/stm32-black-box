#ifndef ISENSOR_H
#define ISENSOR_H

class ISensor {
public:
	virtual ~ISensor() = default;

	virtual bool init() = 0;

	virtual void update() = 0;

	virtual const char* getDataString() = 0;

	virtual const char* getName() = 0;
};

#endif
