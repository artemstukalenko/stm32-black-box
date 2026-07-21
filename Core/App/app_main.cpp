#include "app_main.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include <string.h>

void app_main_task(void *argument) {
	const char* msg = "BlackBox C++ App Started.\r\n";

	HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 100);

	while(1) {
		HAL_GPIO_TogglePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin);

		const char* tick_msg = "Tick...\r\n";

		HAL_UART_Transmit(&huart1, (uint8_t*)tick_msg, strlen(tick_msg), 100);

		osDelay(1000);
	}
}
