#ifndef I2C_INTERFACE_H
#define I2C_INTERFACE_H

#include "HardwareInterface/I2C/II2CInterface.h"
#include "i2c.h"

class I2CInterface : public II2CInterface {

private:
	I2C_HandleTypeDef *hi2c1_;

public:
	I2CInterface(I2C_HandleTypeDef *hi2c1) : hi2c1_(hi2c1) {};
	~I2CInterface() override = default;

	bool memRead(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) override;
	bool memWrite(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) override;

};

#endif
