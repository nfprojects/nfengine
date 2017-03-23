/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Logger class.
 */

#include "PCH.hpp"
#include "nfCommon/Logger/Logger.hpp"


using namespace NFE::Common;

class TestBackend;
class LoggerTest : public testing::Test {};
using LoggerDeathTest = LoggerTest;

/**
* Test backend for testing LoggerBackend class
*/
class TestBackend final : public LoggerBackend
{
public:
    struct LogInfoStruct
    {
        LogType lastType;
        String lastFile;
        int lastLine;
        String lastMsg;
        double lastTime;
    };

    LogInfoStruct mLastLogInfo;

    void Reset() override
    {
        mLastLogInfo = LogInfoStruct();
    }

    void Log(LogType type, const char* srcFile, int line, const char* str,
             double timeElapsed) override
    {
        mLastLogInfo.lastType = type;
        mLastLogInfo.lastFile = String(srcFile);
        mLastLogInfo.lastLine = line;
        mLastLogInfo.lastMsg = String(str);
        mLastLogInfo.lastTime = timeElapsed;
    }
};

class LoggerBackendsTest : public LoggerTest
{
public:
    static void SetUpTestCase()
    {
        /**
         * Test for ListBackends() method needs TestBackend to NOT be registered beforehand.
         * TestBackend will be registered in this test with name="Test". It will be available for every
         * test in LoggerBackendsTest suite.
         */
        std::vector<String> existingBackends = Logger::GetInstance()->ListBackends();

        auto result = std::find(existingBackends.cbegin(), existingBackends.cend(), "Test");
        ASSERT_EQ(existingBackends.cend(), result);

        ASSERT_TRUE(Logger::RegisterBackend("Test", std::make_unique<TestBackend>()));
        existingBackends = Logger::GetInstance()->ListBackends();

        result = std::find(existingBackends.cbegin(), existingBackends.cend(), "Test");
        ASSERT_NE(existingBackends.cend(), result);
        Logger::GetBackend("Test")->Enable(false);
    }
};


/**
 * To verify if this test really passes, one should look at different logs.
 */
TEST_F(LoggerTest, Simple)
{
    LOG_DEBUG("Test log - debug");
    LOG_DEBUG_S("Test stream log - " << "debug");
    LOG_INFO("Test log - info");
    LOG_INFO_S("Test stream log - " << "info");
    LOG_SUCCESS("Test log - success");
    LOG_SUCCESS_S("Test stream log - " << "success");
    LOG_WARNING("Test log - warning");
    LOG_WARNING_S("Test stream log - " << "warning");
    LOG_ERROR("Test log - error");
    LOG_ERROR_S("Test stream log - " << "error");
}

TEST_F(LoggerTest, EscapeSymbols)
{
    LOG_INFO("These are amps && and a <node<>> and some \"\"qoutes\" \'\'in quotes\'\'\".");
    LOG_INFO_S("These are amps && and a <node<>> and some \"\"qoutes\" \'\'in quotes\'\'\".");
}

TEST_F(LoggerTest, Null)
{
    LOG_DEBUG(nullptr);
}

TEST_F(LoggerTest, MultipleTypes)
{
    int num = 5;
    String text = "text";
    float quarter = 1.0f / 4.0f;
    LOG_INFO("String: %s, int: %d, float: %f", text.Str(), num, quarter);
    LOG_INFO_S("String: " << text << ", int: " << num << ", float: " << quarter);
}

TEST_F(LoggerTest, Long)
{
    String longMessage;
    for (int i = 0; i < 25; ++i)
        longMessage += std::to_string(i) + ": There should be 25 this veeery long messages...\n";

    LOG_INFO(longMessage.Str());
    LOG_INFO("%s", longMessage.Str());
}

TEST_F(LoggerTest, Invalid)
{
    // this message won't be printed
    LOG_INFO("This is an invalid format %.s %");
}


// LOGGER BACKENDS

TEST_F(LoggerBackendsTest, DisableAll)
{
    std::vector<String> existingBackends = Logger::GetInstance()->ListBackends();

    LOG_SUCCESS("DisableAll: This log should be seen in every standard backend");

    for (const auto& i : existingBackends)
    {
        LoggerBackend* backend = Logger::GetBackend(i);
        backend->Enable(false);
        ASSERT_FALSE(backend->IsEnabled());
    }

    LOG_ERROR("DisableAll: This log should NOT be seen anywhere!");

    // Turn back on all backends
    for (const auto& i : existingBackends)
    {
        LoggerBackend* backend = Logger::GetBackend(i);
        backend->Enable(true);
        ASSERT_TRUE(backend->IsEnabled());
    }
}

TEST_F(LoggerBackendsTest, DisableSingle)
{
    std::vector<String> existingBackends = Logger::GetInstance()->ListBackends();

    LOG_SUCCESS("DisableSingle: This log should be seen in every standard backend");

    for (const auto& i : existingBackends)
    {
        LoggerBackend* backend = Logger::GetBackend(i);

        backend->Enable(false);
        ASSERT_FALSE(backend->IsEnabled());

        LOG_SUCCESS_S("DisableSingle: This log should NOT be seen in " << i << " backend.");

        backend->Enable(true);
        ASSERT_TRUE(backend->IsEnabled());
    }
}

TEST_F(LoggerBackendsTest, LogMethodArguments)
{
    // Disable all backends apart the test one
    std::vector<String> existingBackends = Logger::GetInstance()->ListBackends();
    for (const auto& i : existingBackends)
    {
        if (i.compare("Test") == 0)
            Logger::GetBackend(i)->Enable(true);
        else
            Logger::GetBackend(i)->Enable(false);
    }

    ASSERT_NE(nullptr, Logger::GetBackend("Test"));

    LOG_INFO("This is log");
    auto lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend("Test"))->mLastLogInfo;
    ASSERT_EQ(lastLogInfo.lastLine, __LINE__ - 2);
    ASSERT_EQ(lastLogInfo.lastMsg, "This is log");
    ASSERT_EQ(lastLogInfo.lastType, LogType::Info);
    ASSERT_EQ(lastLogInfo.lastFile, __FILE__);
    ASSERT_NE(lastLogInfo.lastTime, 0);

    LOG_WARNING_S("This is log2");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend("Test"))->mLastLogInfo;
    ASSERT_EQ(lastLogInfo.lastLine, __LINE__ - 2);
    ASSERT_EQ(lastLogInfo.lastMsg, "This is log2");
    ASSERT_EQ(lastLogInfo.lastType, LogType::Warning);
    ASSERT_EQ(lastLogInfo.lastFile, __FILE__);
    ASSERT_NE(lastLogInfo.lastTime, 0);

    LOG_ERROR("This is log3");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend("Test"))->mLastLogInfo;
    ASSERT_EQ(lastLogInfo.lastLine, __LINE__ - 2);
    ASSERT_EQ(lastLogInfo.lastMsg, "This is log3");
    ASSERT_EQ(lastLogInfo.lastType, LogType::Error);
    ASSERT_EQ(lastLogInfo.lastFile, __FILE__);
    ASSERT_NE(lastLogInfo.lastTime, 0);

    LOG_SUCCESS_S("This is log4");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend("Test"))->mLastLogInfo;
    ASSERT_EQ(lastLogInfo.lastLine, __LINE__ - 2);
    ASSERT_EQ(lastLogInfo.lastMsg, "This is log4");
    ASSERT_EQ(lastLogInfo.lastType, LogType::OK);
    ASSERT_EQ(lastLogInfo.lastFile, __FILE__);
    ASSERT_NE(lastLogInfo.lastTime, 0);

    Logger::GetBackend("Test")->Enable(false);
    Logger::GetBackend("Test")->Reset();
    LOG_INFO("This is log");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend("Test"))->mLastLogInfo;
    ASSERT_EQ(lastLogInfo.lastLine, 0);
    ASSERT_EQ(lastLogInfo.lastMsg, "");
    ASSERT_EQ(lastLogInfo.lastType, static_cast<LogType>(0));
    ASSERT_EQ(lastLogInfo.lastFile, "");
    ASSERT_EQ(lastLogInfo.lastTime, 0);

    // Turn back all backends apart the test one
    for (const auto& i : existingBackends)
    {
        if (i.compare("Test") == 0)
            Logger::GetBackend(i)->Enable(false);
        else
            Logger::GetBackend(i)->Enable(true);
    }
}