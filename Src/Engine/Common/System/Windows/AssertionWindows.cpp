/**
 * @file
 * @author  Witek902
 * @brief   Fatal assertion definitions
 */

#include "PCH.hpp"
#include "../Assertion.hpp"
#include "../Console.hpp"
#include "Math/Math.hpp"

#include <dbghelp.h>

namespace NFE {
namespace Common {

NFE_FORCE_NOINLINE void PrintCallstack(CONTEXT* context, int32 numFunctionsToSkip)
{
    DWORD machine = IMAGE_FILE_MACHINE_AMD64;

    HANDLE process = ::GetCurrentProcess();
    HANDLE thread = ::GetCurrentThread();

    ::SymInitialize(process, NULL, TRUE);
    ::SymSetOptions(SYMOPT_LOAD_LINES);

    STACKFRAME frame = {};
    frame.AddrPC.Offset =       context->Rip;
    frame.AddrPC.Mode =         AddrModeFlat;
    frame.AddrFrame.Offset =    context->Rbp;
    frame.AddrFrame.Mode =      AddrModeFlat;
    frame.AddrStack.Offset =    context->Rsp;
    frame.AddrStack.Mode =      AddrModeFlat;

    int32 skippedFunctions = 0;
    while (::StackWalk64(machine, process, thread, &frame, context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
    {
        // extract data

        uint64 functionAddress;
        const char* moduleName = nullptr;
        const char* functionName = nullptr;
        const char* srcFileName = nullptr;
        uint32 line = UINT32_MAX;
        uint64 displacement = 0;

        functionAddress = frame.AddrPC.Offset;

        DWORD64 moduleBase = ::SymGetModuleBase64(process, frame.AddrPC.Offset);
        char moduleBuff[MAX_PATH];
        if (moduleBase && ::GetModuleFileNameA((HINSTANCE)moduleBase, moduleBuff, MAX_PATH))
        {
            moduleName = moduleBuff;

            const char* croppedModuleName = moduleName;
            while (moduleName && (*moduleName != '\0'))
            {
                if (*moduleName == '\\')
                {
                    croppedModuleName = moduleName + 1;
                }
                moduleName++;
            }
            moduleName = croppedModuleName;
        }

        char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255];
        PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
        symbol->SizeOfStruct = (sizeof IMAGEHLP_SYMBOL) + 255;
        symbol->MaxNameLength = 254;
        
        if (::SymGetSymFromAddr64(process, frame.AddrPC.Offset, &displacement, symbol))
        {
            functionName = symbol->Name;
        }

        DWORD offset = 0;
        IMAGEHLP_LINE srcLine;
        srcLine.SizeOfStruct = sizeof(IMAGEHLP_LINE);
        if (::SymGetLineFromAddr64(process, frame.AddrPC.Offset, &offset, &srcLine))
        {
            srcFileName = srcLine.FileName;
            line = srcLine.LineNumber;

            const char* croppedFileName = srcFileName;
            while (srcFileName && (*srcFileName != '\0'))
            {
                if (*srcFileName == '\\')
                {
                    croppedFileName = srcFileName + 1;
                }
                srcFileName++;
            }
            srcFileName = croppedFileName;
        }

        if (numFunctionsToSkip >= skippedFunctions)
        {
            if (srcFileName)
            {
                PrintColored(ConsoleColor::Cyan,
                    "  %s!%s+0x%llX (%s:%u)\n",
                    moduleName ? moduleName : "<unknown module>",
                    functionName ? functionName : "<unknown function>", displacement,
                    srcFileName, line);
            }
            else
            {
                PrintColored(ConsoleColor::Cyan,
                    "  %s!%s+0x%llX\n",
                    moduleName ? moduleName : "<unknown module>",
                    functionName ? functionName : "<unknown function>", displacement);
            }
        }
        else
        {
            skippedFunctions++;
        }
    }

    SymCleanup(process);
}

NFE_FORCE_NOINLINE void PrintCallstack(int32 numFunctionsToSkip)
{
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_FULL;
    ::RtlCaptureContext(&context);

    PrintCallstack(&context, numFunctionsToSkip);
}

static const char* ExceptionCodeToString(DWORD exceptionCode)
{
    switch (exceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:            return "Access violation";
    case EXCEPTION_DATATYPE_MISALIGNMENT:       return "Datatype misalignment";
    case EXCEPTION_BREAKPOINT:                  return "Breakpoint";
    case EXCEPTION_SINGLE_STEP:                 return "Single step";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:       return "Array bounds exceeded";
    case EXCEPTION_FLT_DENORMAL_OPERAND:        return "Floating-point denormal operand";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:          return "Floating-point division by zero";
    case EXCEPTION_FLT_INEXACT_RESULT:          return "Floating-point inexact result";
    case EXCEPTION_FLT_INVALID_OPERATION:       return "Floating-point invalid operation";
    case EXCEPTION_FLT_OVERFLOW:                return "Floating-point overflow";
    case EXCEPTION_FLT_STACK_CHECK:             return "Floating-point stack check";
    case EXCEPTION_FLT_UNDERFLOW:               return "Floating-point underflow";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:          return "Integer division by zero";
    case EXCEPTION_INT_OVERFLOW:                return "Integer overflow";
    case EXCEPTION_PRIV_INSTRUCTION:            return "Privileged instruction";
    case EXCEPTION_IN_PAGE_ERROR:               return "In page error";
    case EXCEPTION_ILLEGAL_INSTRUCTION:         return "Illegal instruction";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:    return "Noncontinuable exception";
    case EXCEPTION_STACK_OVERFLOW:              return "Stack overflow";
    case EXCEPTION_INVALID_DISPOSITION:         return "Invalid disposition";
    case EXCEPTION_GUARD_PAGE:                  return "Guard page";
    case EXCEPTION_INVALID_HANDLE:              return "Invalid handle";
    case CONTROL_C_EXIT:                        return "Ctrl+C exit";
    default:
        return "Unrecognized Exception";
    }
}

static LONG UnhandledExceptionHandler(_EXCEPTION_POINTERS* exceptionInfo)
{
    const EXCEPTION_RECORD* exceptionRecord = exceptionInfo->ExceptionRecord;

    const char* exceptionCodeStr = ExceptionCodeToString(exceptionRecord->ExceptionCode);

    PrintColored(ConsoleColor::Red, "NFEngine fatal error!\n");

    if ((exceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || exceptionRecord->ExceptionCode == EXCEPTION_IN_PAGE_ERROR) && exceptionRecord->NumberParameters == 2)
    {
        const ULONG opcode = (ULONG)exceptionRecord->ExceptionInformation[0];

        if (opcode == 0)
        {
            PrintColored(ConsoleColor::Red, "Access violation reading location 0x%p\n", exceptionRecord->ExceptionInformation[1]);
        }
        else if (opcode == 8)
        {
            PrintColored(ConsoleColor::Red, "Access violation DEP at location 0x%p\n", exceptionRecord->ExceptionInformation[1]);
        }
        else
        {
            PrintColored(ConsoleColor::Red, "Access violation writing location 0x%p\n", exceptionRecord->ExceptionInformation[1]);
        }
    }
    else
    {
        PrintColored(ConsoleColor::Red, "Exception code: 0x%0X (%s)\n", exceptionRecord->ExceptionCode, exceptionCodeStr);
    }

    PrintColored(ConsoleColor::Red, "Exception address: 0x%p\n", exceptionRecord->ExceptionAddress);

    fflush(stdout);

    PrintColored(ConsoleColor::White, "Callstack:\n");
    PrintCallstack(exceptionInfo->ContextRecord, 0);
    fflush(stdout);

    return EXCEPTION_EXECUTE_HANDLER;
}

void InitFatalErrorHandlers()
{
    ::SetUnhandledExceptionFilter(UnhandledExceptionHandler);
}

bool IsDebuggerAttached()
{
    return ::IsDebuggerPresent() != 0;
}


} // namespace Common
} // namespace NFE
