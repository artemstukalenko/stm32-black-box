#include "Logger/Impl/UartLogger.h"
#include <string.h>

UartLogger::UartLogger(UART_HandleTypeDef* huart) : uartHandle_(huart) {}

bool UartLogger::init() {
	const char* message = "[LOG] UartLogger initialized.\r\n";
	HAL_UART_Transmit(uartHandle_, (uint8_t*) message, strlen(message), 100);
	return true;
}

bool UartLogger::writeLog(const char* message) {
	if (message == nullptr) return false;

	return HAL_UART_Transmit(uartHandle_, (uint8_t*) message, strlen(message), 100) == HAL_OK;
}

bool UartLogger::sync() {
	//do nothing
	return true;
}
