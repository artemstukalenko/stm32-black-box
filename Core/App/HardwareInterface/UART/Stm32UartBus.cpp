#include "HardwareInterface/UART/Stm32UartBus.h"

bool Stm32UartBus::receive(uint8_t *pData, uint16_t Size) {
	return HAL_UART_Receive_IT(huart_, pData, 1) == HAL_OK;
}

bool Stm32UartBus::transmit(uint8_t *pData, uint16_t size, uint32_t timeout) {
	return HAL_UART_Transmit(huart_, pData, size, timeout) == HAL_OK;
}
