#ifndef UARTLOGGER_H
#define UARTLOGGER_H

#include "Logger/ILogger.h"
#include "usart.h"

class UartLogger : public ILogger {

private:
	UART_HandleTypeDef* uartHandle_;

public:
	explicit UartLogger(UART_HandleTypeDef* huart);
	~UartLogger() override = default;

	bool init() override;
	bool writeLog(const char* message) override;
	bool sync() override;
};

#endif
