#ifndef STM32_UART_BUS_H
#define STM32_UART_BUS_H

#include "HardwareInterface/UART/IUartBus.h"
#include "usart.h"

class Stm32UartBus : public IUartBus {

private:
	UART_HandleTypeDef *huart_;

public:
	Stm32UartBus(UART_HandleTypeDef *huart) : huart_(huart) {};
	~Stm32UartBus() override = default;

	bool receive(uint8_t *pData, uint16_t Size) override;

};

#endif
