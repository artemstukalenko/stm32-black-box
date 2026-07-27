#ifndef MOCK_I2CBUS_H
#define MOCK_I2CBUS_H

#include <gmock/gmock.h>
#include "HardwareInterface/I2C/II2CBus.h"

class MockI2CBus : public II2CBus {

public:
	MOCK_METHOD(bool, memRead, (uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout), (override));
	MOCK_METHOD(bool, memWrite, (uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout), (override));

};

#endif
