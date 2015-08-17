#include "PCH.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/Timer.hpp"

using namespace NFE::Common;

TEST(LoggerTest, Simple)
{
    LOG_DEBUG("Test log - debug");
    LOG_INFO("Test log - info");
    LOG_SUCCESS("Test log - success");
    LOG_WARNING("Test log - warning");
    LOG_ERROR("Test log - error");
    LOG_FATAL("Test log - fatal");
}

TEST(LoggerTest, Long)
{
    std::string longMessage;
    for (int i = 0; i < 100; ++i)
        longMessage += "This is a long message... ";
    LOG_DEBUG(longMessage.c_str());
}
