/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Unit tests for Logger class.
 */

#include "PCH.hpp"
#include "../nfCommon/Logger.hpp"

using namespace NFE::Common;

/**
 * To verify if this test really passes, one should look at console and HTML logs.
 */
TEST(LoggerTest, Simple)
{
    ASSERT_NO_THROW(LOG_DEBUG("Test log - debug"));
    ASSERT_NO_THROW(LOG_INFO("Test log - info"));
    ASSERT_NO_THROW(LOG_SUCCESS("Test log - success"));
    ASSERT_NO_THROW(LOG_WARNING("Test log - warning"));
    ASSERT_NO_THROW(LOG_ERROR("Test log - error"));
    ASSERT_NO_THROW(LOG_FATAL("Test log - fatal"));
}

TEST(LoggerTest, Null)
{
    ASSERT_NO_THROW(LOG_DEBUG(nullptr));
}

TEST(LoggerTest, Long)
{
    std::string longMessage;
    for (int i = 0; i < 50; ++i)
        longMessage += std::to_string(i) + ": There should be 50 this veeery long messages...\n";

    ASSERT_NO_THROW(LOG_INFO(longMessage.c_str()));
}

TEST(LoggerTest, Invalid)
{
    ASSERT_NO_THROW(LOG_INFO("This is an invalid format %.s %"));
}
