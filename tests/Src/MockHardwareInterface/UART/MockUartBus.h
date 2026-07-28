#ifndef MOCK_UART_BUS_H
#define MOCK_UART_BUS_H

#include <gmock/gmock.h>
#include "HardwareInterface/UART/IUartBus.h"

class MockUartBus : public IUartBus {
public:
	MOCK_METHOD(bool, receive, (uint8_t *pData, uint16_t Size), (override));
};

#endif
