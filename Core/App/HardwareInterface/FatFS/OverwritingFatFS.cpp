#include "HardwareInterface/FatFS/OverwritingFatFS.h"

bool OverwritingFatFS::mount() {
	return f_mount(&fs_, "", 0) == FR_OK;
}

bool OverwritingFatFS::close() {
	return f_close(&file_) == FR_OK;
}

bool OverwritingFatFS::open() {
	return f_open(&file_, fileName_, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK;
}

bool OverwritingFatFS::write(const char* msg) {
	UINT bytesWritten = 0;
	size_t messageLength = strlen(msg);
	return (f_write(&file_, msg, messageLength, &bytesWritten) == FR_OK)
			&& bytesWritten == messageLength;
}

bool OverwritingFatFS::sync() {
	return f_sync(&file_) == FR_OK;
}
