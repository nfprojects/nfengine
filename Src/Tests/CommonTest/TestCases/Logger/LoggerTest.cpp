/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Logger class.
 */

#include "PCH.hpp"
#include "Engine/Common/Logger/LoggerImpl.hpp"
#include "Engine/Common/Containers/StringView.hpp"


using namespace NFE::Common;

class TestBackend;
class LoggerTest : public testing::Test {};
using LoggerDeathTest = LoggerTest;

namespace {

const StringView testBackendName("Test");

} // namespace

/**
* Test backend for testing ILoggerBackend class
*/
class TestBackend final : public ILoggerBackend
{
public:
    struct LogInfoStruct
    {
        LogType lastType;
        std::string lastFile;
        int lastLine;
        std::string lastMsg;
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
        mLastLogInfo.lastFile = std::string(srcFile);
        mLastLogInfo.lastLine = line;
        mLastLogInfo.lastMsg = std::string(str);
        mLastLogInfo.lastTime = timeElapsed;
    }
};

class LoggerBackendsTest : public LoggerTest
{
public:
    static void SetUpTestCase()
    {
        ASSERT_TRUE(Logger::RegisterBackend(testBackendName, MakeUniquePtr<TestBackend>()));

        ILoggerBackend* testBackend = Logger::GetBackend(testBackendName);
        ASSERT_NE(nullptr, testBackend);
        testBackend->Enable(false);
    }
};


/**
 * To verify if this test really passes, one should look at different logs.
 */
TEST_F(LoggerTest, Simple)
{
    NFE_LOG_DEBUG("Test log - debug");
    NFE_LOG_DEBUG_S("Test stream log - " << "debug");
    NFE_LOG_INFO("Test log - info");
    NFE_LOG_INFO_S("Test stream log - " << "info");
    NFE_LOG_SUCCESS("Test log - success");
    NFE_LOG_SUCCESS_S("Test stream log - " << "success");
    NFE_LOG_WARNING("Test log - warning");
    NFE_LOG_WARNING_S("Test stream log - " << "warning");
    NFE_LOG_ERROR("Test log - error");
    NFE_LOG_ERROR_S("Test stream log - " << "error");
}

TEST_F(LoggerTest, EscapeSymbols)
{
    NFE_LOG_INFO("These are amps && and a <node<>> and some \"\"qoutes\" \'\'in quotes\'\'\".");
    NFE_LOG_INFO_S("These are amps && and a <node<>> and some \"\"qoutes\" \'\'in quotes\'\'\".");
}

TEST_F(LoggerTest, Null)
{
    NFE_LOG_DEBUG(nullptr);
}

TEST_F(LoggerTest, MultipleTypes)
{
    int num = 5;
    std::string text = "text";
    float quarter = 1.0f / 4.0f;
    NFE_LOG_INFO("String: %s, int: %d, float: %f", text.c_str(), num, quarter);
    NFE_LOG_INFO_S("String: " << text << ", int: " << num << ", float: " << quarter);
}

TEST_F(LoggerTest, Long)
{
    std::string longMessage;
    for (int i = 0; i < 25; ++i)
        longMessage += std::to_string(i) + ": There should be 25 this veeery long messages...\n";

    NFE_LOG_INFO(longMessage.c_str());
    NFE_LOG_INFO("%s", longMessage.c_str());
}

TEST_F(LoggerTest, Invalid)
{
    // this message won't be printed
    NFE_LOG_INFO("This is an invalid format %.s %");
}


// LOGGER BACKENDS

TEST_F(LoggerBackendsTest, DisableAll)
{
    const LoggerBackendMap& existingBackends = Logger::GetInstance()->ListBackends();

    NFE_LOG_SUCCESS("DisableAll: This log should be seen in every standard backend");

    for (const auto& backend : existingBackends)
    {
        backend.ptr->Enable(false);
        ASSERT_FALSE(backend.ptr->IsEnabled());
    }

    NFE_LOG_ERROR("DisableAll: This log should NOT be seen anywhere!");

    // Turn back on all backends
    for (const auto& backend : existingBackends)
    {
        backend.ptr->Enable(true);
        ASSERT_TRUE(backend.ptr->IsEnabled());
    }
}

TEST_F(LoggerBackendsTest, DisableSingle)
{
    const LoggerBackendMap& existingBackends = Logger::GetInstance()->ListBackends();

    NFE_LOG_SUCCESS("DisableSingle: This log should be seen in every standard backend");

    for (const auto& backend : existingBackends)
    {
        backend.ptr->Enable(false);
        ASSERT_FALSE(backend.ptr->IsEnabled());

        NFE_LOG_SUCCESS("DisableSingle: This log should NOT be seen in '%s' backend.", backend.name.Str());

        backend.ptr->Enable(true);
        ASSERT_TRUE(backend.ptr->IsEnabled());
    }
}

TEST_F(LoggerBackendsTest, LogMethodArguments)
{
    // Disable all backends apart the test one
    const LoggerBackendMap& existingBackends = Logger::GetInstance()->ListBackends();
    for (const auto& backend : existingBackends)
    {
        if (backend.name == testBackendName)
            backend.ptr->Enable(true);
        else
            backend.ptr->Enable(false);
    }

    ASSERT_NE(nullptr, Logger::GetBackend(testBackendName));

    NFE_LOG_INFO("This is log");
    auto lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend(testBackendName))->mLastLogInfo;
    ASSERT_EQ(lastLogInfo.lastLine, __LINE__ - 2);
    ASSERT_EQ(lastLogInfo.lastMsg, "This is log");
    ASSERT_EQ(lastLogInfo.lastType, LogType::Info);
    ASSERT_EQ(lastLogInfo.lastFile, __FILE__);
    ASSERT_NE(lastLogInfo.lastTime, 0);

    NFE_LOG_WARNING_S("This is log2");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend(testBackendName))->mLastLogInfo;
    ASSERT_EQ(lastLogInfo.lastLine, __LINE__ - 2);
    ASSERT_EQ(lastLogInfo.lastMsg, "This is log2");
    ASSERT_EQ(lastLogInfo.lastType, LogType::Warning);
    ASSERT_EQ(lastLogInfo.lastFile, __FILE__);
    ASSERT_NE(lastLogInfo.lastTime, 0);

    NFE_LOG_ERROR("This is log3");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend(testBackendName))->mLastLogInfo;
    ASSERT_EQ(lastLogInfo.lastLine, __LINE__ - 2);
    ASSERT_EQ(lastLogInfo.lastMsg, "This is log3");
    ASSERT_EQ(lastLogInfo.lastType, LogType::Error);
    ASSERT_EQ(lastLogInfo.lastFile, __FILE__);
    ASSERT_NE(lastLogInfo.lastTime, 0);

    NFE_LOG_SUCCESS_S("This is log4");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend(testBackendName))->mLastLogInfo;
    ASSERT_EQ(lastLogInfo.lastLine, __LINE__ - 2);
    ASSERT_EQ(lastLogInfo.lastMsg, "This is log4");
    ASSERT_EQ(lastLogInfo.lastType, LogType::OK);
    ASSERT_EQ(lastLogInfo.lastFile, __FILE__);
    ASSERT_NE(lastLogInfo.lastTime, 0);

    Logger::GetBackend(testBackendName)->Enable(false);
    Logger::GetBackend(testBackendName)->Reset();
    NFE_LOG_INFO("This is log");
    lastLogInfo = reinterpret_cast<TestBackend*>(Logger::GetBackend(testBackendName))->mLastLogInfo;
    ASSERT_EQ(lastLogInfo.lastLine, 0);
    ASSERT_EQ(lastLogInfo.lastMsg, "");
    ASSERT_EQ(lastLogInfo.lastType, static_cast<LogType>(0));
    ASSERT_EQ(lastLogInfo.lastFile, "");
    ASSERT_EQ(lastLogInfo.lastTime, 0);

    // Turn back all backends apart the test one
    for (const auto& backend : existingBackends)
    {
        if (backend.name == testBackendName)
            backend.ptr->Enable(false);
        else
            backend.ptr->Enable(true);
    }
}