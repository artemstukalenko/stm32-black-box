#ifndef FATFS_LOGGER_H
#define FATFS_LOGGER_H

#include "Logger/ILogger.h"
#include "HardwareInterface/FatFS/IFatFS.h"

class FatFSLogger : public ILogger {

private:
	IFatFS* fatFs_;

public:
	explicit FatFSLogger(IFatFS* fatFs);
	~FatFSLogger() override = default;

	bool init() override;
	bool writeLog(const char* message) override;
	bool sync() override;
};

#endif
