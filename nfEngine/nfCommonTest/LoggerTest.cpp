/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Logger class.
 */

#include "PCH.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/EnumIterator.hpp"

using namespace NFE::Common;

class LoggerTest : public testing::Test {};
using LoggerDeathTest = LoggerTest;
using LoggerBackendsTest = LoggerTest;

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

  /*
// LOGGER BACKENDS TESTS
TEST(LoggerBackendsTest, UnRegistration)
{
    auto logPtr = Logger::GetInstance();
    LOG_SUCCESS("UnRegistration: This should be logged everywhere! No.1!");

    // Unregister all backends
    ASSERT_NO_THROW(logPtr->UnRegisterBackend());
    LOG_SUCCESS("UnRegistration: This should not be logged anywhere! No.1!");

    // Unregistering any backend and all backends once again should pose no problem
    ASSERT_NO_THROW(logPtr->UnRegisterBackend());
    ASSERT_NO_THROW(logPtr->UnRegisterBackend(LoggerBackendType::All));
    for (const auto& i : EnumIterator<LoggerBackendType>())
        ASSERT_NO_THROW(logPtr->UnRegisterBackend(i));

    LOG_SUCCESS("UnRegistration: This should not be logged anywhere! No.2!");
}

TEST(LoggerBackendsTest, Registration)
{
    auto logPtr = Logger::GetInstance();
    LOG_SUCCESS("Registration: This should be logged everywhere! No.1!");

    // Unregister all backends
    ASSERT_NO_THROW(logPtr->UnRegisterBackend());
    LOG_SUCCESS("Registration: This should not be logged anywhere! No.1!");

    // Register all backends
    ASSERT_NO_THROW(logPtr->RegisterBackend());
    LOG_SUCCESS("Registration: This should be logged everywhere! No.2!");

    // Registering any backend and all backends once again should pose no problem
    ASSERT_NO_THROW(logPtr->RegisterBackend());
    ASSERT_NO_THROW(logPtr->RegisterBackend(LoggerBackendType::All));
    for (const auto& i : EnumIterator<LoggerBackendType>())
        ASSERT_NO_THROW(logPtr->RegisterBackend(i));

    LOG_SUCCESS("Registration: This should be logged everywhere! No.3!");
}

TEST(LoggerBackendsTest, Console)
{
    auto logPtr = Logger::GetInstance();
    ASSERT_NO_THROW(logPtr->UnRegisterBackend());
    LOG_SUCCESS("Console: This should not be logged anywhere!");

    ASSERT_NO_THROW(logPtr->RegisterBackend(LoggerBackendType::Console));
    LOG_SUCCESS("Console: This should be logged only in console!");

    ASSERT_NO_THROW(logPtr->UnRegisterBackend(LoggerBackendType::Console));
    LOG_SUCCESS("Console: This should not be logged anywhere, AGAIN!");
}

TEST(LoggerBackendsTest, Txt)
{
    auto logPtr = Logger::GetInstance();
    ASSERT_NO_THROW(logPtr->UnRegisterBackend());
    LOG_SUCCESS("Txt: This should not be logged anywhere!");

    ASSERT_NO_THROW(logPtr->RegisterBackend(LoggerBackendType::Txt));
    LOG_SUCCESS("Txt: This should be logged only in text file!");

    ASSERT_NO_THROW(logPtr->UnRegisterBackend(LoggerBackendType::Txt));
    LOG_SUCCESS("Txt: This should not be logged anywhere, AGAIN!");
}

TEST(LoggerBackendsTest, HTML)
{
    auto logPtr = Logger::GetInstance();
    ASSERT_NO_THROW(logPtr->UnRegisterBackend());
    LOG_SUCCESS("HTML: This should not be logged anywhere!");

    ASSERT_NO_THROW(logPtr->RegisterBackend(LoggerBackendType::HTML));
    LOG_SUCCESS("HTML: This should be logged only in HTML file!");

    ASSERT_NO_THROW(logPtr->UnRegisterBackend(LoggerBackendType::HTML));
    LOG_SUCCESS("HTML: This should not be logged anywhere, AGAIN!");
}

TEST(LoggerBackendsTest, WinDebugger)
{
    auto logPtr = Logger::GetInstance();
    ASSERT_NO_THROW(logPtr->UnRegisterBackend());
    LOG_SUCCESS("WinDebugger: This should not be logged anywhere!");

    // This still won't be visible on any other OSs than Windows!
    ASSERT_NO_THROW(logPtr->RegisterBackend(LoggerBackendType::WinDebugger));
    LOG_SUCCESS("WinDebugger: This should be logged only in Windows Debugger!");

    ASSERT_NO_THROW(logPtr->UnRegisterBackend(LoggerBackendType::WinDebugger));
    LOG_SUCCESS("WinDebugger: This should not be logged anywhere, AGAIN!");
}

TEST(LoggerBackendsTest, All)
{
    auto logPtr = Logger::GetInstance();
    LOG_SUCCESS("All: This should be logged everywhere!");

    ASSERT_NO_THROW(logPtr->UnRegisterBackend());
    LOG_SUCCESS("All: This should not be logged anywhere!");

    ASSERT_NO_THROW(logPtr->RegisterBackend());
    LOG_SUCCESS("All: This should be logged everywhere AGAIN!");
}          */