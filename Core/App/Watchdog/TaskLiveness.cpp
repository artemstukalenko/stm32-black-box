#include "Watchdog/TaskLiveness.h"

void TaskLiveness::markAlive(WatchdogTaskId id, uint32_t nowMs) {
	lastAliveMs_[static_cast<size_t>(id)] = nowMs;
}

bool TaskLiveness::allAlive(uint32_t nowMs, uint32_t staleThresholdMs) {
	for(size_t i = 0; i < static_cast<size_t>(WatchdogTaskId::Count); i++) {
		if (nowMs - lastAliveMs_[i] > staleThresholdMs) {
			return false;
		}
	}
	return true;
}
