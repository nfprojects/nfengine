/**
 * @file
 * @author  Witek902
 * @brief   Fatal assertion declarations
 */

#pragma once

#include "../nfCommon.hpp"


namespace NFE {
namespace Common {

void InitFatalErrorHandlers();
void PrintCallstack(int32 numFunctionsToSkip = 1);

// check if the process is being debugged
NFCOMMON_API bool IsDebuggerAttached();

NFCOMMON_API void ReportFatalAssertion(const char* expressionStr, const char* functionStr, const char* srcFile, int line, const char* str, ...);
NFCOMMON_API void ReportFatalAssertion(const char* expressionStr, const char* functionStr, const char* srcFile, int line);
NFCOMMON_API void HandleFatalAssertion();

} // namespace Common
} // namespace NFE


#ifndef NFE_CONFIGURATION_FINAL

#define NFE_ASSERT(expression, ...)                                                                     \
do {                                                                                                    \
    if (!(expression))                                                                                  \
    {                                                                                                   \
        NFE::Common::ReportFatalAssertion(#expression, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);  \
        if (NFE::Common::IsDebuggerAttached()) NFE_BREAK();                                             \
        NFE::Common::HandleFatalAssertion();                                                            \
    }                                                                                                   \
} while (0)

#define NFE_FATAL(...)                                                                                  \
do {                                                                                                    \
    NFE::Common::ReportFatalAssertion("", __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);               \
    if (NFE::Common::IsDebuggerAttached()) NFE_BREAK();                                                 \
    NFE::Common::HandleFatalAssertion();                                                                \
} while (0)

#else // NFE_CONFIGURATION_FINAL

#define NFE_ASSERT(expression, ...) do { } while (0)

// hint the compiler that this code should never be reached
#ifdef NFE_PLATFORM_WINDOWS
#define NFE_FATAL(...) __assume(0)
#elif defined(NFE_PLATFORM_LINUX)
#define NFE_FATAL(...) __builtin_unreachable()
#else
#error Invalid platform
#endif

#endif // NFE_CONFIGURATION_FINAL