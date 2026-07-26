#include "HardwareInterface/I2C/I2CInterface.h"

bool I2CInterface::memRead(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
	return HAL_I2C_Mem_Read(hi2c1_, DevAddress, MemAddress, MemAddSize, pData, Size, Timeout) == HAL_OK;
}

bool I2CInterface::memWrite(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
	return HAL_I2C_Mem_Write(hi2c1_, DevAddress, MemAddress, MemAddSize, pData, Size, Timeout) == HAL_OK;
}
