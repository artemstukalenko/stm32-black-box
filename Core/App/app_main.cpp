#include "app_main.h"
#include "main.h"
#include "usart.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>

#include "Logger/Impl/UsbCdcLogger.h"
#include "Sensor/ISensor.h"
#include "Sensor/Barometer/Barometer.h"
#include "Sensor/Barometer/MockBarometer.h"
#include "Sensor/GPS/Gps.h"

#define MAX_LOG_MSG_LENGTH 64
#define SENSOR_COUNT 2

extern I2C_HandleTypeDef hi2c1;

struct LogMessage {
	char data[MAX_LOG_MSG_LENGTH];
};

Barometer barometer(&hi2c1);
Gps gps(&huart2);

ISensor* sensors[] = {&barometer, &gps};

UsbCdcLogger logger;

osMessageQueueId_t logQueueHandle;
osThreadId_t sensorTaskHandle;
osThreadId_t loggerTaskHandle;


const osThreadAttr_t sensorTask_attributes = {
		.name = "SensorTask",
		.stack_size = 256 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t loggerTask_attributes = {
		.name = "LoggerTask",
		.stack_size = 256 * 4,
		.priority = (osPriority_t) osPriorityBelowNormal
};


void SensorTask(void* parameters) {
	ISensor* sensor = static_cast<ISensor*>(parameters);
	uint32_t delay = sensor->getDelay();

	for(;;) {
		if (sensor != nullptr) {
			sensor->update();
			const char* dataStr = sensor->getDataString();

			LogMessage message;
			strncpy(message.data, dataStr, MAX_LOG_MSG_LENGTH - 1);
			message.data[MAX_LOG_MSG_LENGTH - 1] = '\0';

			osMessageQueuePut(logQueueHandle, &message, 0, 0);

			osDelay(delay);
		}
	}
}

void LoggerTask(void *argument) {
	LogMessage receviedMessage;

	for(;;) {
		if (osMessageQueueGet(logQueueHandle, &receviedMessage, NULL, osWaitForever) == osOK) {
			logger.writeLog(receviedMessage.data);
		}
	}
}


void app_main_task(void *argument) {
	logger.init();

	for (int i = 0; i < SENSOR_COUNT; i++) {
		char stringBuffer[64];
		const char* messageTemplate = "[SYS] %s %s\r\n";
		if (sensors[i]->init()) {
			snprintf(stringBuffer, sizeof(stringBuffer), messageTemplate,
					sensors[i]->getName(), "initialized.");
			logger.writeLog(stringBuffer);
		} else {
			snprintf(stringBuffer, sizeof(stringBuffer), messageTemplate,
								sensors[i]->getName(), "failed initialization.");
			logger.writeLog(stringBuffer);
		}
	}

	logger.writeLog("[SYS] BlackBox started.\r\n");
	logger.writeLog("*************************\r\n");

	logQueueHandle = osMessageQueueNew(10, sizeof(LogMessage), NULL);

	for (int i = 0; i < SENSOR_COUNT; i++) {
		osThreadNew(SensorTask, sensors[i], &sensorTask_attributes);
	}

	loggerTaskHandle = osThreadNew(LoggerTask, NULL, &loggerTask_attributes);

	while(1) {
		HAL_GPIO_TogglePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin);
		osDelay(100);
		HAL_GPIO_TogglePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin);
		osDelay(1900);
	}
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		gps.handleRxInterrupt();
	}
}
