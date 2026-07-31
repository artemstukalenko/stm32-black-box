#ifndef MOCK_FATFS_H
#define MOCK_FATFS_H

#include <gmock/gmock.h>
#include "HardwareInterface/FatFS/IFatFS.h"

class MockFatFS : public IFatFS {

public:
	MOCK_METHOD(bool, mount, (), (override));
	MOCK_METHOD(bool, close, (), (override));
	MOCK_METHOD(bool, open, (), (override));
	MOCK_METHOD(bool, write, (const char* msg), (override));
	MOCK_METHOD(bool, sync, (), (override));

};

#endif
