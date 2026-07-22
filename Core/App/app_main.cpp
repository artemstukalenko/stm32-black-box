#include "app_main.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"

#include "Sensor/Barometer/MockBarometer.h"
#include "Logger/Impl/MockLogger.h"

MockBarometer barometer;
MockLogger logger(&huart1);

void app_main_task(void *argument) {
	logger.init();

	if (barometer.init()) {
		logger.writeLog("[SYS] Barometer initialized.\r\n");
	} else {
		logger.writeLog("[SYS] Barometer initialization failed.");
	}

	logger.writeLog("[SYS] BlackBox started.\r\n");
	logger.writeLog("*************************\r\n");

	while(1) {
		HAL_GPIO_TogglePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin);

		barometer.update();

		const char* barometerDataString = barometer.getDataString();

		logger.writeLog(barometerDataString);

		osDelay(3000);
	}
}
