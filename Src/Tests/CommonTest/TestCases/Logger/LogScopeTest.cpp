/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Unit tests for LogScope class.
 */

#include "PCH.hpp"
#include "Engine/Common/Logger/LogScope.hpp"
#include "Engine/Common/Logger/LoggerImpl.hpp"



using namespace NFE::Common;

namespace {

const StringView testBackendName("Test");

const char* scopeText = "Entering scope ";
const char* scopeExitText = "Exiting scope ";

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

class LogScopeTest : public testing::Test
{
public:
    TestBackend* mBackend;
    void SetUp()
    {
        // Ensure that the test backend is registered and enabled
        Logger::RegisterBackend(testBackendName, MakeUniquePtr<TestBackend>());
        mBackend = dynamic_cast<TestBackend*>(Logger::GetBackend(testBackendName));
        ASSERT_NE(nullptr, mBackend);

        const LoggerBackendMap& backends = Logger::ListBackends();
        for (const auto& backend : backends)
        {
            if (backend.name == testBackendName)
                backend.ptr->Enable(true);
            else
                backend.ptr->Enable(false);
        }

        mBackend->Reset();
    }

    void TearDown()
    {
        ASSERT_NE(nullptr, mBackend);
        mBackend->Enable(false);
    }
};


TEST_F(LogScopeTest, EmptyConstructor)
{
    // Lambda for checking if anything was logged
    auto wasLogged = [this]() -> bool {
        if (mBackend == nullptr)
            return false;

        bool result = !mBackend->mLastLogInfo.lastFile.empty();
        return result;
    };

    const char* scopeName = "Scope";
    std::stringstream outputLog;
    {
        ASSERT_FALSE(wasLogged());

        LogScope scope;
        ASSERT_TRUE(wasLogged());

        outputLog << scopeText << scopeName << ".";
        ASSERT_EQ(0, mBackend->mLastLogInfo.lastMsg.compare(outputLog.str()))
                << "Output log was: " << outputLog.str();
        mBackend->Reset();
        outputLog.str("");

        ASSERT_FALSE(wasLogged());
    }
    ASSERT_TRUE(wasLogged());

    outputLog << scopeExitText << scopeName << ".";
    ASSERT_EQ(0, mBackend->mLastLogInfo.lastMsg.compare(outputLog.str()))
            << "Output log was: " << outputLog.str();
    mBackend->Reset();
}

TEST_F(LogScopeTest, SimpleConstructor)
{
    // Lambda for checking if anything was logged
    auto wasLogged = [this]() -> bool {
        if (mBackend == nullptr)
            return false;

        bool result = !mBackend->mLastLogInfo.lastFile.empty();
        return result;
    };

    const char* scopeName = "Awesome Scope";
    std::stringstream outputLog;

    {
        ASSERT_FALSE(wasLogged());

        LogScope scope(scopeName);
        ASSERT_TRUE(wasLogged());

        outputLog << scopeText << scopeName << ".";
        ASSERT_EQ(0, mBackend->mLastLogInfo.lastMsg.compare(outputLog.str()))
                << "Output log was: " << outputLog.str();
        mBackend->Reset();
        outputLog.str("");

        ASSERT_FALSE(wasLogged());
    }
    ASSERT_TRUE(wasLogged());

    outputLog << scopeExitText << scopeName << ".";
    ASSERT_EQ(0, mBackend->mLastLogInfo.lastMsg.compare(outputLog.str()))
            << "Output log was: " << outputLog.str();
    mBackend->Reset();
}

TEST_F(LogScopeTest, MacroConstructor)
{
    // Lambda for checking if anything was logged
    auto wasLogged = [this]() -> bool {
        if (mBackend == nullptr)
            return false;

        bool result = !mBackend->mLastLogInfo.lastFile.empty();
        return result;
    };

    const char* scopeName = "AwesomeScope";
    std::stringstream outputLog;

    {
        ASSERT_FALSE(wasLogged());

        LOG_SCOPE(AwesomeScope);
        ASSERT_TRUE(wasLogged());

        outputLog << scopeText << scopeName << " in " <<  __FILE__ << "@" << (__LINE__ - 3) << ".";
        ASSERT_EQ(0, mBackend->mLastLogInfo.lastMsg.compare(outputLog.str()))
                << "Output log was: " << outputLog.str();
        mBackend->Reset();
        outputLog.str("");

        ASSERT_FALSE(wasLogged());
    }
    ASSERT_TRUE(wasLogged());

    outputLog << scopeExitText << scopeName << ".";
    ASSERT_EQ(0, mBackend->mLastLogInfo.lastMsg.compare(outputLog.str()))
            << "Output log was: " << outputLog.str();
    mBackend->Reset();
}
