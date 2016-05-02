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

/**
 * To verify if this test really passes, one should look at console and HTML logs.
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

TEST(LoggerTest, XMLEscapeSeqs)
{
    ASSERT_NO_THROW(LOG_INFO("These are amps && and a <node<>> and some \"\"qoutes\" \'\'in quotes\'\'\"."));
    ASSERT_NO_THROW(LOG_INFO_S("These are amps && and a <node<>> and some \"\"qoutes\" \'\'in quotes\'\'\"."));
}

TEST(LoggerDeathTest, FatalLogs)
{
    ASSERT_EXIT(LOG_FATAL("Some fatal log."), testing::ExitedWithCode(1), "");
    ASSERT_EXIT(LOG_FATAL_S("Some fatal log."), testing::ExitedWithCode(1), "");
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
