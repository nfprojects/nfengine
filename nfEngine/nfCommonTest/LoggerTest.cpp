/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Logger class.
 */

#include "PCH.hpp"
#include "../nfCommon/Logger.hpp"

using namespace NFE::Common;

class LoggerTest : public testing::Test {};
using LoggerDeathTest = LoggerTest;
using LoggerBackendsTest = LoggerTest;

struct LogInfoStruct
{
    LogType lastType;
    std::string lastFile;
    int lastLine;
    std::string lastMsg;
    double lastTime;
};

class TestBackend : public LoggerBackend
{
    LogInfoStruct mLastLogInfo;

public:
    void Reset()
    {
        mLastLogInfo = LogInfoStruct();
    }

    void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed)
    {
        mLastLogInfo.lastType = type;
        mLastLogInfo.lastFile = std::string(srcFile);
        mLastLogInfo.lastLine = line;
        mLastLogInfo.lastMsg = std::string(str);
        mLastLogInfo.lastTime = timeElapsed;
    };

    LogInfoStruct GetLastLogInfo()
    {
        return mLastLogInfo;
    }
};


/**
 * To verify if this test really passes, one should look at different logs.
 */
TEST(LoggerTest, Simple)
{
    ASSERT_NO_THROW(LOG_DEBUG("Test log - debug"));
    ASSERT_NO_THROW(LOG_DEBUG_S("Test stream log - " << "debug"));
    ASSERT_NO_THROW(LOG_INFO("Test log - info"));
    ASSERT_NO_THROW(LOG_INFO_S("Test stream log - " << "info"));
    ASSERT_NO_THROW(LOG_SUCCESS("Test log - success"));
    ASSERT_NO_THROW(LOG_SUCCESS_S("Test stream log - " << "success"));
    ASSERT_NO_THROW(LOG_WARNING("Test log - warning"));
    ASSERT_NO_THROW(LOG_WARNING_S("Test stream log - " << "warning"));
    ASSERT_NO_THROW(LOG_ERROR("Test log - error"));
    ASSERT_NO_THROW(LOG_ERROR_S("Test stream log - " << "error"));
}

TEST(LoggerTest, Null)
{
    ASSERT_NO_THROW(LOG_DEBUG(nullptr));
}

TEST(LoggerTest, MultipleTypes)
{
    int num = 5;
    std::string text = "text";
    float quarter = 1.0f / 4.0f;
    ASSERT_NO_THROW(LOG_INFO("String: %s, int: %d, float: %f", text.c_str(), num, quarter));
    ASSERT_NO_THROW(LOG_INFO_S("String: " << text << ", int: " << num << ", float: " << quarter));
}

TEST(LoggerTest, Long)
{
    std::string longMessage;
    for (int i = 0; i < 25; ++i)
        longMessage += std::to_string(i) + ": There should be 25 this veeery long messages...\n";

    ASSERT_NO_THROW(LOG_INFO(longMessage.c_str()));
    ASSERT_NO_THROW(LOG_INFO("%s", longMessage.c_str()));
}

TEST(LoggerTest, Invalid)
{
    // this message won't be printed
    ASSERT_NO_THROW(LOG_INFO("This is an invalid format %.s %"));
}


// LOGGER DEATHTESTS
TEST(LoggerDeathTest, FatalLogs)
{
    ASSERT_EXIT(LOG_FATAL("Some fatal log."), testing::ExitedWithCode(1), "");
    ASSERT_EXIT(LOG_FATAL_S("Some fatal log."), testing::ExitedWithCode(1), "");
}


// LOGGER BACKENDS
TEST(LoggerBackendsTest, ListBackends)
{
    std::vector<std::string> existingBackends = Logger::GetInstance()->ListBackends();

    auto result = std::find(existingBackends.begin(), existingBackends.end(), "Test");
    ASSERT_EQ(existingBackends.end(), result);

    Logger::RegisterBackend("Test", new TestBackend);
    existingBackends = Logger::GetInstance()->ListBackends();

    result = std::find(existingBackends.begin(), existingBackends.end(), "Test");
    ASSERT_NE(existingBackends.end(), result);
}

TEST(LoggerBackendsTest, DisableAll)
{
    std::vector<std::string> existingBackends = Logger::GetInstance()->ListBackends();

    LOG_SUCCESS("DisableAll: This log should be seen in every standard backend");

    for (auto& i : existingBackends)
    {
        LoggerBackend* backend = Logger::GetBackend(i);
        ASSERT_TRUE(backend->IsEnabled());
        backend->Enable(false);
        ASSERT_FALSE(backend->IsEnabled());
    }

    LOG_ERROR("DisableAll: This log should NOT be seen anywhere!");

    // Turn back on all backends
    for (auto& i : existingBackends)
    {
        LoggerBackend* backend = Logger::GetBackend(i);
        ASSERT_FALSE(backend->IsEnabled());
        backend->Enable(true);
        ASSERT_TRUE(backend->IsEnabled());
    }
}

TEST(LoggerBackendsTest, DisableSingle)
{
    std::vector<std::string> existingBackends = Logger::GetInstance()->ListBackends();

    LOG_SUCCESS("DisableSingle: This log should be seen in every standard backend");

    for (auto& i : existingBackends)
    {
        LoggerBackend* backend = Logger::GetBackend(i);
        ASSERT_TRUE(backend->IsEnabled());

        backend->Enable(false);
        ASSERT_FALSE(backend->IsEnabled());

        LOG_SUCCESS_S("DisableSingle: This log should NOT be seen in " << i << " backend.");

        backend->Enable(true);
        ASSERT_TRUE(backend->IsEnabled());
    }
}

TEST(LoggerBackendsTest, LogMethodArguments)
{
    // Disable all backends apart the test one
    std::vector<std::string> existingBackends = Logger::GetInstance()->ListBackends();
    for (auto& i : existingBackends)
    {
        if (i != "Test")
            Logger::GetBackend(i)->Enable(false);
    }

    LogInfoStruct lastLogInfo;
    LOG_INFO("This is log");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend("Test"))->GetLastLogInfo();
    ASSERT_EQ(lastLogInfo.lastLine, __LINE__ - 2);
    ASSERT_EQ(lastLogInfo.lastMsg, "This is log");
    ASSERT_EQ(lastLogInfo.lastType, LogType::Info);
    ASSERT_EQ(lastLogInfo.lastFile, __FILE__);
    ASSERT_NE(lastLogInfo.lastTime, 0);

    LOG_WARNING_S("This is log2");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend("Test"))->GetLastLogInfo();
    ASSERT_EQ(lastLogInfo.lastLine, __LINE__ - 2);
    ASSERT_EQ(lastLogInfo.lastMsg, "This is log2");
    ASSERT_EQ(lastLogInfo.lastType, LogType::Warning);
    ASSERT_EQ(lastLogInfo.lastFile, __FILE__);
    ASSERT_NE(lastLogInfo.lastTime, 0);

    LOG_ERROR("This is log3");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend("Test"))->GetLastLogInfo();
    ASSERT_EQ(lastLogInfo.lastLine, __LINE__ - 2);
    ASSERT_EQ(lastLogInfo.lastMsg, "This is log3");
    ASSERT_EQ(lastLogInfo.lastType, LogType::Error);
    ASSERT_EQ(lastLogInfo.lastFile, __FILE__);
    ASSERT_NE(lastLogInfo.lastTime, 0);

    LOG_SUCCESS_S("This is log4");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend("Test"))->GetLastLogInfo();
    ASSERT_EQ(lastLogInfo.lastLine, __LINE__ - 2);
    ASSERT_EQ(lastLogInfo.lastMsg, "This is log4");
    ASSERT_EQ(lastLogInfo.lastType, LogType::OK);
    ASSERT_EQ(lastLogInfo.lastFile, __FILE__);
    ASSERT_NE(lastLogInfo.lastTime, 0);

    Logger::GetBackend("Test")->Enable(false);
    Logger::GetBackend("Test")->Reset();
    LOG_INFO("This is log");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend("Test"))->GetLastLogInfo();
    ASSERT_EQ(lastLogInfo.lastLine, 0);
    ASSERT_EQ(lastLogInfo.lastMsg, "");
    ASSERT_EQ(lastLogInfo.lastType, static_cast<LogType>(0));
    ASSERT_EQ(lastLogInfo.lastFile, "");
    ASSERT_EQ(lastLogInfo.lastTime, 0);

    // Turn back all backends apart the test one
    for (auto& i : existingBackends)
    {
        if (i == "Test")
            Logger::GetBackend(i)->Enable(false);
        else
            Logger::GetBackend(i)->Enable(true);
    }
}