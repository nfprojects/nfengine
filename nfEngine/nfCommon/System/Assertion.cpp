/**
 * @file
 * @author  Witek902 (witek902@gmail.com)
 * @brief   Fatal assertion definitions
 */

#include "PCH.hpp"
#include "Assertion.hpp"

#if defined(__LINUX__) | defined(__linux__)
#include <signal.h>
#endif // defined(__LINUX__) | defined(__linux__)


namespace NFE {
namespace Common {

void HandleFatalAssertion(const char* expressionStr, const char* functionStr, const char* srcFile, int line, const char* str, ...)
{
    // keep shorter strings on the stack
    const int SHORT_MESSAGE_LENGTH = 4096;
    char stackBuffer[SHORT_MESSAGE_LENGTH];

    std::unique_ptr<char[]> buffer;
    char* formattedStr = nullptr;
    va_list args, argsCopy;
    va_start(args, str);
    va_copy(argsCopy, args);

    int len = vsnprintf(stackBuffer, SHORT_MESSAGE_LENGTH, str, args);
    if (len < 0)
    {
        va_end(argsCopy);
        va_end(args);
    }

    if (len >= SHORT_MESSAGE_LENGTH)  // buffer on the stack is too small
    {
        buffer.reset(new (std::nothrow) char[len + 1]);
        if (buffer)
        {
            formattedStr = buffer.get();
            vsnprintf(formattedStr, len + 1, str, argsCopy);
        }
    }
    else if (len > 0)  // buffer on the stack is sufficient
    {
        formattedStr = stackBuffer;
    }

    va_end(argsCopy);
    va_end(args);

    std::stringstream message;
    message << "NFEngine assertion '" << expressionStr << "' failed.\n\n";
    message << "Source file: " << srcFile << ':' << line << "\n";
    message << "Function: " << functionStr << "\n";
    message << "Message: " << (formattedStr ? formattedStr : "(vsnprintf failed)");

#if defined(WIN32)
    if (::IsDebuggerPresent())
    {
        ::DebugBreak();
    }
    else
    {
        message << "\n\nPress 'YES' to continue (not recommended) or 'NO' to abort.";
        if (IDYES == ::MessageBoxA(NULL, message.str().Str(), "Assertion failed", MB_ICONERROR | MB_YESNO))
        {
            return;
        }
    }

    ::ExitProcess(1);
#elif defined(__LINUX__) | defined(__linux__)
    ::raise(SIGTRAP);
#endif // defined(WIN32)
}

} // namespace Common
} // namespace NFE
