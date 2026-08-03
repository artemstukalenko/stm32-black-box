#include "Logger/Impl/LoggerStrategy.h"

LoggerStrategy::LoggerStrategy(FatFSLogger* fatFsLogger, UsbCdcLogger* usbCdcLogger)
	: fatFsLogger_(fatFsLogger), usbCdcLogger_(usbCdcLogger) {}

bool LoggerStrategy::init() {
	usbCdcLogger_->init();
	return fatFsLogger_->init();
}

bool LoggerStrategy::writeLog(const char* message) {
	usbCdcLogger_->writeLog(message);
	return fatFsLogger_->writeLog(message);
}

bool LoggerStrategy::sync() {
	usbCdcLogger_->sync();
	return fatFsLogger_->sync();
}
