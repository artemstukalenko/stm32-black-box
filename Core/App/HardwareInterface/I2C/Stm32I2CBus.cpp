#include <HardwareInterface/I2C/Stm32I2CBus.h>

bool Stm32I2CBus::memRead(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
	return HAL_I2C_Mem_Read(hi2c1_, DevAddress, MemAddress, MemAddSize, pData, Size, Timeout) == HAL_OK;
}

bool Stm32I2CBus::memWrite(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout) {
	return HAL_I2C_Mem_Write(hi2c1_, DevAddress, MemAddress, MemAddSize, pData, Size, Timeout) == HAL_OK;
}
