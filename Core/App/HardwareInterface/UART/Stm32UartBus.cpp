#include "HardwareInterface/UART/Stm32UartBus.h"

bool Stm32UartBus::receive(uint8_t *pData, uint16_t Size) {
	return HAL_UART_Receive_IT(huart_, pData, 1) == HAL_OK;
}
