#include "Logger/Impl/UsbCdcLogger.h"
#include "usbd_cdc_if.h"
#include <string.h>

bool UsbCdcLogger::init() {
	writeLog("[LOG] UsbCdcLogger initialized.\r\n");
	return true;
}

bool UsbCdcLogger::writeLog(const char* message) {
	if (message == nullptr) return false;

	CDC_Transmit_FS((uint8_t*)message, ((uint16_t) strlen(message)));
	return true;
}

bool UsbCdcLogger::sync() {
	//do nothing
	return true;
}
