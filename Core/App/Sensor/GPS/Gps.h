#ifndef GPS_H
#define GPS_H

#include "Sensor/ISensor.h"
#include "stm32f4xx_hal.h"

class Gps : public ISensor {

private:
	UART_HandleTypeDef* huart_;
	char rxByte_;
	char sentenceBuffer[128];
	uint8_t sentenceIndex_;
	char stringBuffer[128];
	bool newDataAvailable_;

	void feedData(uint8_t byte);

public:
	explicit Gps(UART_HandleTypeDef* huart);
	~Gps() override = default;

	bool init() override;
	void update() override;
	const char* getDataString() override;
	const char* getName() override;
	uint32_t getDelay() override;

	void handleRxInterrupt();

};

#endif
