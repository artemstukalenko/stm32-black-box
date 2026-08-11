#include "app_main.h"
#include "main.h"
#include "usart.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>

#include <HardwareInterface/I2C/Stm32I2CBus.h>
#include <HardwareInterface/UART/Stm32UartBus.h>
#include <HardWareInterface/FatFS/OverwritingFatFS.h>

#include "Logger/Impl/FatFSLogger.h"
#include "Logger/Impl/UsbCdcLogger.h"
#include "Logger/Impl/LoggerStrategy.h"
#include "Sensor/ISensor.h"
#include "Sensor/Barometer/Barometer.h"
#include "Sensor/GPS/Gps.h"

#define MAX_LOG_MSG_LENGTH 96
#define SENSOR_COUNT 2

extern I2C_HandleTypeDef hi2c1;

struct LogMessage {
	char source[24];
	char data[MAX_LOG_MSG_LENGTH];
};

Stm32I2CBus i2cBus(&hi2c1);
Barometer barometer(&i2cBus);

Stm32UartBus uartBus(&huart2);
Gps gps(&uartBus);

ISensor* sensors[] = {&barometer, &gps};

OverwritingFatFS fatfs;
FatFSLogger fatFsLogger(&fatfs);
UsbCdcLogger usbCdcLogger;
LoggerStrategy logger(&fatFsLogger, &usbCdcLogger);

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

const osThreadAttr_t mavLinkTask_attributes = {
		.name = "MavLinkTask",
		.stack_size = 256 * 4,
		.priority = (osPriority_t) osPriorityBelowNormal
};

void SensorTask(void* parameters) {
	ISensor* sensor = static_cast<ISensor*>(parameters);
	uint32_t delay = sensor->getDelay();

	for(;;) {
		if (sensor != nullptr) {
			sensor->update();
			const char* dataStr;

			while ((dataStr = sensor->getDataString())[0] != '\0') {
				LogMessage message;
				const char* sensorName = sensor->getName();
				strncpy(message.source, sensorName, sizeof(message.source) - 1);
				message.source[sizeof(message.source) - 1] = '\0';
				strncpy(message.data, dataStr, MAX_LOG_MSG_LENGTH - 1);
				message.data[MAX_LOG_MSG_LENGTH - 1] = '\0';
				osMessageQueuePut(logQueueHandle, &message, 0, 0);
			}

			osDelay(delay);
		}
	}
}

void LoggerTask(void *argument) {
	LogMessage receviedMessage;

	for(;;) {
		if (osMessageQueueGet(logQueueHandle, &receviedMessage, NULL, osWaitForever) == osOK) {
			char stringBuffer[64];
 			const char* sensorLoggingTemplate = "[%s]: %s";
			snprintf(stringBuffer, sizeof(stringBuffer), sensorLoggingTemplate, receviedMessage.source, receviedMessage.data);
			bool writeResult = logger.writeLog(stringBuffer);
			bool syncResult = logger.sync();

			if (!writeResult || !syncResult) {
				HAL_GPIO_WritePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin, GPIO_PIN_RESET);
			}
		}
	}
}

void MavLinkTask(void *argument) {
	for (;;) {
		BarometerReading barometerReading = barometer.getReading();
		char stringBuffer[128];
		const char* barometerReadingTemplate = "BarometerReading: pressure = %d Pa, temperature = %d C\r\n";
		snprintf(stringBuffer, sizeof(stringBuffer), barometerReadingTemplate, (int) barometerReading.pressurePa, (int) barometerReading.temperatureC);
		logger.writeLog(stringBuffer);
		osDelay(3000);

		GpsReading gpsReading = gps.getReading();
		const char* gpsReadingTemplate = "GpsReading: latitude =  %d, longitude = %d, utcTimeOfFix = %d, fixValid = %d\r\n";
		snprintf(stringBuffer, sizeof(stringBuffer), gpsReadingTemplate, (int) gpsReading.latitude, (int) gpsReading.longitude, (int) gpsReading.utcTimeOfFix,
				gpsReading.fixType);
		logger.writeLog(stringBuffer);
		osDelay(3000);
	}
}

void app_main_task(void *argument) {
	if (logger.init()) {
		logger.writeLog("[SYS] Logger initialized.");
	}

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
	logger.sync();

	logQueueHandle = osMessageQueueNew(32, sizeof(LogMessage), NULL);

	for (int i = 0; i < SENSOR_COUNT; i++) {
		osThreadNew(SensorTask, sensors[i], &sensorTask_attributes);
	}

	loggerTaskHandle = osThreadNew(LoggerTask, NULL, &loggerTask_attributes);

	osThreadNew(MavLinkTask, NULL, &mavLinkTask_attributes);

	HAL_GPIO_TogglePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin);
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		gps.handleRxInterrupt();
	}
}
