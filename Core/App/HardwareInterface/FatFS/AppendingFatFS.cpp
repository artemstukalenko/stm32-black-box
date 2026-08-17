#include <HardwareInterface/FatFS/AppendingFatFS.h>

bool AppendingFatFS::mount() {
	return f_mount(&fs_, "", 0) == FR_OK;
}

bool AppendingFatFS::close() {
	return f_close(&file_) == FR_OK;
}

bool AppendingFatFS::open() {
	return f_open(&file_, fileName_, FA_OPEN_APPEND | FA_WRITE) == FR_OK;
}

bool AppendingFatFS::write(const char* msg) {
	UINT bytesWritten = 0;
	size_t messageLength = strlen(msg);
	return (f_write(&file_, msg, messageLength, &bytesWritten) == FR_OK)
			&& bytesWritten == messageLength;
}

bool AppendingFatFS::sync() {
	return f_sync(&file_) == FR_OK;
}
