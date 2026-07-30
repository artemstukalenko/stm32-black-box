#ifndef USBCDCLOGGER_H
#define USBCDCLOGGER_H

#pragma once

#include "Logger/ILogger.h"
#include <cstdint>

class UsbCdcLogger : public ILogger {

public:
	UsbCdcLogger() = default;
	~UsbCdcLogger() override = default;

	bool init() override;
	bool writeLog(const char* message) override;
	bool sync() override;

};

#endif
