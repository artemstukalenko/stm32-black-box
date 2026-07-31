#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Logger/Impl/FatFSLogger.h"
#include "MockHardwareInterface/FatFS/MockFatFS.h"

using ::testing::_;
using ::testing::Return;

class FatFSLoggerTest : public ::testing::Test {

protected:
	MockFatFS fatFs;
	FatFSLogger* logger;

	void SetUp() override {
		logger = new FatFSLogger(&fatFs);
	}

	void TearDown() override {
		delete logger;
	}

};


TEST_F(FatFSLoggerTest, InitSuccess) {
	EXPECT_CALL(fatFs, mount())
			.WillOnce(Return(true));

	EXPECT_CALL(fatFs, open())
			.WillOnce(Return(true));

	EXPECT_TRUE(logger->init());
}

TEST_F(FatFSLoggerTest, InitFailsWhenMountFails) {
	EXPECT_CALL(fatFs, mount())
			.WillOnce(Return(false));
	EXPECT_CALL(fatFs, open())
			.Times(0);

	EXPECT_FALSE(logger->init());
}

TEST_F(FatFSLoggerTest, InitFailsWhenOpenFails) {
	EXPECT_CALL(fatFs, mount())
				.WillOnce(Return(true));

	EXPECT_CALL(fatFs, open())
				.WillOnce(Return(false));

	EXPECT_FALSE(logger->init());
}

TEST_F(FatFSLoggerTest, WriteLogSuccess) {
	const char* msg = "Test log message";

	EXPECT_CALL(fatFs, write(msg))
		.WillOnce(Return(true));

	EXPECT_TRUE(logger->writeLog(msg));
}

TEST_F(FatFSLoggerTest, WriteLogFailsWhenFatFSWriteFails) {
	const char* msg = "Test log message";

	EXPECT_CALL(fatFs, write(msg))
	.WillOnce(Return(false));

	EXPECT_FALSE(logger->writeLog(msg));
}

TEST_F(FatFSLoggerTest, WriteLogHandlesNull) {
	EXPECT_CALL(fatFs, write(_)).Times(0);

	EXPECT_FALSE(logger->writeLog(nullptr));
}

TEST_F(FatFSLoggerTest, SyncSuccess) {
	EXPECT_CALL(fatFs, sync())
			.WillOnce(Return(true));

	EXPECT_TRUE(logger->sync());
}

TEST_F(FatFSLoggerTest, SyncFailsWhenFsSyncFails) {
	EXPECT_CALL(fatFs, sync())
			.WillOnce(Return(false));

	EXPECT_FALSE(logger->sync());
}
