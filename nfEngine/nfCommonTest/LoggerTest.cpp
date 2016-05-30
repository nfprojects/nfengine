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
    // We get the list of the standard nfCommon backends and we create the anticipated set
    std::vector<std::string> existingBackends = Logger::GetInstance()->ListBackends();
    std::vector<std::string> plannedBackends = {"Console", "HTML", "TXT"};
#ifdef WIN32
    plannedBackends.push_back("WinDebugger");
#endif

    // We create the container for the difference set of these 2 vectors
    std::vector<std::string> difference;

    std::sort(existingBackends.begin(), existingBackends.end());
    std::sort(plannedBackends.begin(), plannedBackends.end());

    // Differences vector should be empty now - initialized
    ASSERT_TRUE(difference.empty());
    std::set_difference(existingBackends.begin(), existingBackends.end(),
                        plannedBackends.begin(), plannedBackends.end(),
                        std::back_inserter(difference));

    // Differences vector should be empty now as well - no differences
    ASSERT_TRUE(difference.empty());
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