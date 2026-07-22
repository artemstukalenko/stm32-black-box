#ifndef MOCKLOGGER_H
#define MOCKLOGGER_H

#include "Logger/ILogger.h"
#include "usart.h"

class MockLogger : public ILogger {

private:
	UART_HandleTypeDef* uartHandle;

public:
	explicit MockLogger(UART_HandleTypeDef* huart);
	~MockLogger() override = default;

	bool init() override;
	bool writeLog(const char* message) override;
	void sync() override;
};

#endif
