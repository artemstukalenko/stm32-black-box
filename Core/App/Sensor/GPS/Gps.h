#ifndef GPS_H
#define GPS_H

#include "Sensor/ISensor.h"
#include "HardwareInterface/UART/Stm32UartBus.h"

class Gps : public ISensor {

private:
	Stm32UartBus* uartBus_;
	char rxByte_;
	char sentenceBuffer[128];
	uint8_t sentenceIndex_;
	char stringBuffer[128];
	bool newDataAvailable_;

	void feedData(uint8_t byte);

public:
	explicit Gps(Stm32UartBus* uartBus_);
	~Gps() override = default;

	bool init() override;
	bool update() override;
	const char* getDataString() override;
	const char* getName() override;
	uint32_t getDelay() override;

	void handleRxInterrupt();

};

#endif
