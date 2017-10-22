/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Fatal assertion declarations
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace Common {

void NFCOMMON_API HandleFatalAssertion(const char* expressionStr, const char* functionStr, const char* srcFile,
                                       int line, const char* str, ...);

} // namespace Common
} // namespace NFE


#if defined(WIN32)
#define NFE_DEBUG_BREAK() ::DebugBreak()
#else
#define NFE_DEBUG_BREAK()
#endif


// TODO assertions should be disabled in "Final" build
#define NFE_ASSERT(expression, ...)                                                                     \
do {                                                                                                    \
    if (!(expression))                                                                                  \
    {                                                                                                   \
        NFE_DEBUG_BREAK();                                                                              \
        NFE::Common::HandleFatalAssertion(#expression, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);  \
    }                                                                                                   \
} while (0)
