#ifndef II2C_BUS_H
#define II2C_BUS_H

#include <cstdint>

class II2CBus {

public:
	virtual ~II2CBus() = default;
	virtual bool memRead(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) = 0;
	virtual bool memWrite(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) = 0;
};

#endif
