#ifndef IUARTBUS_H
#define IUARTBUS_H

#include <cstdint>

class IUartBus {

public:
	virtual ~IUartBus() = default;

	virtual bool receive(uint8_t *pData, uint16_t Size) = 0;
};

#endif
