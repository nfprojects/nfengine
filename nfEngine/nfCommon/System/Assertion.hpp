/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Fatal assertion declarations
 */

#pragma once

#include "../nfCommon.hpp"
#include "../Logger/Logger.hpp"


namespace NFE {
namespace Common {

void NFCOMMON_API HandleFatalAssertion(const char* expressionStr, const char* functionStr, const char* srcFile,
                                       int line, const char* str, ...);

} // namespace Common
} // namespace NFE


// TODO assertions should be disabled in "Final" build
#define NFE_ASSERT(expression, ...)                                                                     \
do {                                                                                                    \
    if (!(expression))                                                                                  \
    {                                                                                                   \
        LOG_FATAL(__VA_ARGS__);                                                                         \
        NFE::Common::HandleFatalAssertion(#expression, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);  \
    }                                                                                                   \
} while (0)
