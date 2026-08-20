#ifndef APPENDING_FATFS_H
#define APPENDING_FATFS_H

#include "HardwareInterface/FatFS/IFatFS.h"
#include "ff.h"
#include <string.h>

class AppendingFatFS : public IFatFS {

private:
	FATFS fs_;
	FIL file_;
	const char* fileName_ = "bb_log.txt";

public:
	~AppendingFatFS() override = default;

	bool mount() override;
	bool close() override;
	bool open() override;
	bool write(const char* msg) override;
	bool sync() override;
};

#endif
