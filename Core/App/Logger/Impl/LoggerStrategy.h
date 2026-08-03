#ifndef LOGGER_STRATEGY_H
#define LOGGER_STRATEGY_H

#include "Logger/ILogger.h"
#include "Logger/Impl/FatFSLogger.h"
#include "Logger/Impl/UsbCdcLogger.h"

class LoggerStrategy : public ILogger {

private:
	FatFSLogger* fatFsLogger_;
	UsbCdcLogger* usbCdcLogger_;

public:
	explicit LoggerStrategy(FatFSLogger* fatFsLogger, UsbCdcLogger* usbCdcLogger);
	~LoggerStrategy() override = default;

	bool init() override;
	bool writeLog(const char* message) override;
	bool sync() override;

};

#endif
