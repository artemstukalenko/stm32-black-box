#include "Logger/Impl/FatFSLogger.h"

FatFSLogger::FatFSLogger(IFatFS* fatFs) : fatFs_(fatFs) {}

bool FatFSLogger::init() {
	if (!fatFs_->mount()) {
		return false;
	}
	return fatFs_->open();
}

bool FatFSLogger::writeLog(const char* message) {
	if (message == nullptr) return false;
	return fatFs_->write(message);
}

bool FatFSLogger::sync() {
	return fatFs_->sync();
}
