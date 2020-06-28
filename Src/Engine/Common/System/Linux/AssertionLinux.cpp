/**
 * @file
 * @author  Witek902
 * @brief   Fatal assertion definitions
 */

#include "PCH.hpp"
#include "../Assertion.hpp"
#include "Console.hpp"

#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>


namespace NFE {
namespace Common {

NFE_FORCE_NOINLINE void PrintCallstack(int32 numFunctionsToSkip)
{
    constexpr uint32 MaxNumFunctions = 128;

    // based on: https://panthema.net/2008/0901-stacktrace-demangled/

    // storage array for stack trace address data
    void* addrlist[MaxNumFunctions + 1];

    // retrieve current stack addresses
    int32 addrlen = ::backtrace(addrlist, sizeof(addrlist) / sizeof(void*));
    if (addrlen == 0)
    {
	    return;
    }

    // resolve addresses into strings containing "filename(function+address)", this array must be free()-ed
    char** symbollist = ::backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int32 i = numFunctionsToSkip; i < addrlen; i++)
    {
        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbollist[i]; *p; ++p)
        {
            if (*p == '(')
            {
                begin_name = p;
            }
            else if (*p == '+')
            {
                begin_offset = p;
            }
            else if (*p == ')' && begin_offset)
            {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset && begin_name < begin_offset)
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int32 status;
            char* ret = ::abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
            if (status == 0)
            {
                funcname = ret; // use possibly realloc()-ed string
                PrintColored(ConsoleColor::Cyan, "  %s!%s+%s\n", symbollist[i], funcname, begin_offset);
            }
            else
            {
                // demangling failed. Output function name as a C function with no arguments.
                PrintColored(ConsoleColor::Cyan, "  %s!%s()+%s\n", symbollist[i], begin_name, begin_offset);
            }
        }
        else
        {
            // couldn't parse the line? print the whole line.
            PrintColored(ConsoleColor::Cyan, "  %s\n", symbollist[i]);
        }
    }

    free(funcname);
    free(symbollist);
}

NFE_FORCE_NOINLINE static void ReportFatalError(const char* message)
{
    PrintColored(ConsoleColor::Red, "NFEngine fatal error!\n");
    PrintColored(ConsoleColor::Red, "Message: %s\n", message);
    fflush(stdout);

    PrintColored(ConsoleColor::White, "Callstack:\n");
    PrintCallstack(2); // skipping 2 functions: PrintCallstack and this one (ReportFatalAssertion)
    fflush(stdout);
}

static void SignalHandler(int sig)
{
    switch (sig)
    {
    case SIGSEGV:   ReportFatalError("Memory access violation"); break;
    case SIGFPE:    ReportFatalError("Floating point error (or division by zero)"); break;
    case SIGILL:    ReportFatalError("Illegal instruction"); break;
    }

    HandleFatalAssertion();
}

void InitFatalErrorHandlers()
{
    struct sigaction action = {};
    action.sa_handler = &SignalHandler;

    sigaction(SIGSEGV, &action, nullptr);
    sigaction(SIGFPE, &action, nullptr);
    sigaction(SIGILL, &action, nullptr);
}

bool IsDebuggerAttached()
{
    return false;

    char buf[4096];

    const int status_fd = ::open("/proc/self/status", O_RDONLY);
    if (status_fd == -1)
    {
        return false;
    }

    const ssize_t num_read = ::read(status_fd, buf, sizeof(buf) - 1);
    if (num_read <= 0)
    {
        return false;
    }

    buf[num_read] = '\0';
    constexpr char tracerPidString[] = "TracerPid:";
    const char* tracer_pid_ptr = ::strstr(buf, tracerPidString);
    if (!tracer_pid_ptr)
    {
        return false;
    }

    bool isDebuggerAttached = false;
    for (const char* characterPtr = tracer_pid_ptr + sizeof(tracerPidString) - 1; characterPtr <= buf + num_read; ++characterPtr)
    {
        if (::isspace(*characterPtr))
        {
            continue;
        }
        else
        {
            isDebuggerAttached = ::isdigit(*characterPtr) != 0 && *characterPtr != '0';
            break;
        }
    }

    return isDebuggerAttached;
}

} // namespace Common
} // namespace NFE
