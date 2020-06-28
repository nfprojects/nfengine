/**
 * @file
 * @author  Witek902
 * @brief   Fatal assertion definitions
 */

#include "PCH.hpp"
#include "Assertion.hpp"
#include "Console.hpp"


namespace NFE {
namespace Common {

void ReportFatalAssertion(const char* expressionStr, const char* functionStr, const char* srcFile, int32 line)
{
    ReportFatalAssertion(expressionStr, functionStr, srcFile, line, "<no message>");
}

void ReportFatalAssertion(const char* expressionStr, const char* functionStr, const char* srcFile, int32 line, const char* str, ...)
{
    // keep shorter strings on the stack
    const int32 SHORT_MESSAGE_LENGTH = 4096;
    char stackBuffer[SHORT_MESSAGE_LENGTH];

    std::unique_ptr<char[]> buffer;
    char* formattedStr = nullptr;
    va_list args, argsCopy;
    va_start(args, str);
    va_copy(argsCopy, args);

    int32 len = vsnprintf(stackBuffer, SHORT_MESSAGE_LENGTH, str, args);
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

    PrintColored(ConsoleColor::Red, "NFEngine assertion failed!\n");
    PrintColored(ConsoleColor::Red, "Expression: %s\n", expressionStr);
    PrintColored(ConsoleColor::Red, "Source file: %s:%u\n", srcFile, line);
    PrintColored(ConsoleColor::Red, "Function: %s\n", functionStr);
    PrintColored(ConsoleColor::Red, "Message: %s\n", formattedStr ? formattedStr : "(vsnprintf failed)");
    fflush(stdout);

    PrintColored(ConsoleColor::White, "Callstack:\n");
    PrintCallstack(2); // skipping 2 functions: PrintCallstack and this one (ReportFatalAssertion)
    fflush(stdout);
}

static volatile bool gThisIsAlwaysTrue = true;

void HandleFatalAssertion()
{
    const int32 assertExitCode = 2;

    if (gThisIsAlwaysTrue)
    {
#if defined(WIN32)
        ::ExitProcess(assertExitCode);
#elif defined(__LINUX__) | defined(__linux__)
        ::exit(assertExitCode);
#endif // defined(WIN32)
    }
}

} // namespace Common
} // namespace NFE
