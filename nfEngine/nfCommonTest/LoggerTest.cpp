#include "PCH.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/Timer.hpp"

using namespace NFE::Common;

TEST(LoggerTest, Simple)
{
    LOG_INFO("Test log - info");
    LOG_SUCCESS("Test log - success");
    LOG_WARNING("Test log - warning");
    LOG_ERROR("Test log - error");
    LOG_FATAL("Test log - fatal");
}
