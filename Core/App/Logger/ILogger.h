#ifndef ILOGGER_H
#define ILOGGER_H

#include <stdint.h>

class ILogger {
public:
	virtual ~ILogger() = default;

	virtual bool init() = 0;

	virtual bool writeLog(const char* message) = 0;

	virtual bool sync() = 0;
};

#endif
