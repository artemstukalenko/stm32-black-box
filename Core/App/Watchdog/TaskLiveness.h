#ifndef APP_WATCHDOG_TASKLIVENESS_H_
#define APP_WATCHDOG_TASKLIVENESS_H_

#include <cstdint>
#include <cstddef>

enum class WatchdogTaskId {
	Barometer,
	Gps,
	Logger,
	MavLink,
	Count
};


class TaskLiveness {
public:
	void markAlive(WatchdogTaskId id, uint32_t nowMs);
	bool allAlive(uint32_t nowMs, uint32_t staleThresholdMs);

private:
	volatile uint32_t lastAliveMs_[static_cast<size_t>(WatchdogTaskId::Count)] = {0};
};


#endif
