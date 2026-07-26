#ifndef STM32_I2C_BUS_H
#define STM32_I2C_BUS_H

#include <HardwareInterface/I2C/II2CBus.h>
#include "i2c.h"

class Stm32I2CBus : public II2CBus {

private:
	I2C_HandleTypeDef *hi2c1_;

public:
	Stm32I2CBus(I2C_HandleTypeDef *hi2c1) : hi2c1_(hi2c1) {};
	~Stm32I2CBus() override = default;

	bool memRead(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) override;
	bool memWrite(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) override;

};

#endif
