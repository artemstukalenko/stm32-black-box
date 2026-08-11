#ifndef IUARTBUS_H
#define IUARTBUS_H

#include <cstdint>

class IUartBus {

public:
	virtual ~IUartBus() = default;

	virtual bool receive(uint8_t *pData, uint16_t Size) = 0;

	virtual bool transmit(uint8_t *pData, uint16_t size, uint32_t timeout) = 0;
};

#endif
