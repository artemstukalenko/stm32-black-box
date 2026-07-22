#include "MockLogger.h"
#include <string.h>

MockLogger::MockLogger(UART_HandleTypeDef* huart) : uartHandle(huart) {}

bool MockLogger::init() {
	const char* message = "[LOG] MockLogger initialized.\r\n";
	HAL_UART_Transmit(uartHandle, (uint8_t*) message, strlen(message), 100);
	return true;
}

bool MockLogger::writeLog(const char* message) {
	if (message == nullptr) return false;

	HAL_UART_Transmit(uartHandle, (uint8_t*) message, strlen(message), 100);
	return true;
}

void MockLogger::sync() {
	//do nothing
}
