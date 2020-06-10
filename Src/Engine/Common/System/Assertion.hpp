/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Fatal assertion declarations
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace Common {

void NFCOMMON_API HandleFatalAssertion(const char* expressionStr, const char* functionStr, const char* srcFile, int line, const char* str, ...);
void NFCOMMON_API HandleFatalAssertion(const char* expressionStr, const char* functionStr, const char* srcFile, int line);

} // namespace Common
} // namespace NFE


#ifndef NFE_CONFIGURATION_FINAL

#define NFE_ASSERT(expression, ...)                                                                     \
do {                                                                                                    \
    if (!(expression))                                                                                  \
    {                                                                                                   \
        NFE_BREAK();                                                                                    \
        NFE::Common::HandleFatalAssertion(#expression, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);  \
    }                                                                                                   \
} while (0)

#define NFE_FATAL(...)                                                                                  \
do {                                                                                                    \
    NFE_BREAK();                                                                                        \
    NFE::Common::HandleFatalAssertion("", __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);               \
} while (0)

#else // NFE_CONFIGURATION_FINAL

#define NFE_ASSERT(expression, ...) do { } while (0)

#define NFE_FATAL(...) do { } while (0)

#endif // NFE_CONFIGURATION_FINAL