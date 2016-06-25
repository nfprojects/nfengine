/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Logger class.
 */

#include "PCH.hpp"
#include "nfCommon/LogGrouper.hpp"

using namespace NFE::Common;


/**
* Test backend for testing LoggerBackend class
*/
class TestBackend final : public LoggerBackend
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

class LogGrouperTest : public testing::Test
{
    void SetUp()
    {
        // Ensure that the test backend is registered and enabled
        Logger::RegisterBackend("Test", std::make_unique<TestBackend>());
        auto testBackend = dynamic_cast<TestBackend*>(Logger::GetBackend("Test"));
        ASSERT_NE(nullptr, testBackend);

        auto backends = Logger::ListBackends();
        for (const auto& b : backends)
        {
            if (b.compare("Test") == 0)
                Logger::GetBackend(b)->Enable(true);
            else
                Logger::GetBackend(b)->Enable(false);
        }

        testBackend->Reset();
    }

    void TearDown()
    {
        auto testBackend = dynamic_cast<TestBackend*>(Logger::GetBackend("Test"));
        ASSERT_NE(nullptr, testBackend);
        testBackend->Enable(false);
    }
};


TEST_F(LogGrouperTest, Simple)
{
    // Lambda for checking if anything was logged
    auto wasLogged = []() -> bool {
        auto tBackend = Logger::GetBackend("Test");
        if (tBackend == nullptr)
            return false;
        return !dynamic_cast<TestBackend*>(tBackend)->mLastLogInfo.lastFile.empty();
    };

    auto testBackend = dynamic_cast<TestBackend*>(Logger::GetBackend("Test"));
    ASSERT_NE(nullptr, testBackend);

    double loggerTimeBeg, loggerTimeEnd;

    {
        LOG_GROUP_CREATE(testGroup);
        ASSERT_FALSE(wasLogged());

        LOG_INFO_GROUP(testGroup, "some log %d", 43);
        ASSERT_FALSE(wasLogged());

        LOG_INFO_S_GROUP(testGroup, "some log " << 43);
        ASSERT_FALSE(wasLogged());

        LOG_DEBUG_GROUP(testGroup, "some log %d", 43);
        ASSERT_FALSE(wasLogged());

        LOG_WARNING_GROUP(testGroup, "some log %d", 43);
        ASSERT_FALSE(wasLogged());

        LOG_INFO("some log");
        loggerTimeBeg = testBackend->mLastLogInfo.lastTime;
        testBackend->Reset();
        ASSERT_FALSE(wasLogged());

        LOG_ERROR_GROUP(testGroup, "some log %d", 43);
        ASSERT_FALSE(wasLogged());

        LOG_INFO("some log");
        loggerTimeEnd = testBackend->mLastLogInfo.lastTime;
        testBackend->Reset();
        ASSERT_FALSE(wasLogged());
    }
    ASSERT_TRUE(wasLogged());

    auto logInfo = testBackend->mLastLogInfo;
    ASSERT_EQ(__FILE__, logInfo.lastFile);
    ASSERT_EQ("some log 43", logInfo.lastMsg);
    ASSERT_EQ(LogType::Error, logInfo.lastType);
    ASSERT_LT(loggerTimeBeg, logInfo.lastTime);
    ASSERT_GT(loggerTimeEnd, logInfo.lastTime);

    testBackend->Enable(false);
}

TEST_F(LogGrouperTest, Fatal)
{
    // Lambda for checking if anything was logged
    auto wasLogged = []() -> bool {
        auto tBackend = Logger::GetBackend("Test");
        if (tBackend == nullptr)
            return false;
        return !dynamic_cast<TestBackend*>(tBackend)->mLastLogInfo.lastFile.empty();
    };

    auto testBackend = dynamic_cast<TestBackend*>(Logger::GetBackend("Test"));
    ASSERT_NE(nullptr, testBackend);

    {
        LOG_GROUP_CREATE(testGroup);
        ASSERT_FALSE(wasLogged());

        LOG_INFO_GROUP(testGroup, "some log 43");
        ASSERT_FALSE(wasLogged());

        // If it exits - it means that LogGrouper was flushed and quit
        ASSERT_EXIT(LOG_FATAL_GROUP(testGroup, "some log %d", 43), testing::ExitedWithCode(1), "");

        // If it exits - it means that LogGrouper was flushed and quit
        ASSERT_EXIT(LOG_FATAL_S_GROUP(testGroup, "some log " << 43), testing::ExitedWithCode(1), "");

        LOG_INFO_GROUP(testGroup, "some log 43");
        ASSERT_FALSE(wasLogged());
    }
    // Fatal log should flush all logs before it - new ones should be valid and wait for next flush
    ASSERT_TRUE(wasLogged());
}