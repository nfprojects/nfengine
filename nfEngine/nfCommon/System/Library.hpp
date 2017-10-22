/**
 * @file
 * @author Mkkulagowski (mkkulagowski(at)gmail.com)
 * @author Witek902 (witek902@gmail.com)
 * @brief  Library class declaration.
 */

#pragma once

#include "../nfCommon.hpp"

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // WIN32


namespace NFE {
namespace Common {

/**
 * Class used to load dynamic libraries.
 */
class NFCOMMON_API Library
{
private:
#if defined(WIN32)
    HMODULE mModule;
#elif defined(__LINUX__) | defined(__linux__)
    void* mModule;
#else
#error "Target system not supported!"
#endif // WIN32

    void* GetSymbol(const String& name);

public:
    Library();
    Library(const StringView path);
    Library(const Library& other) = delete;
    Library(Library&& other);
    Library& operator=(const Library& other) = delete;
    Library& operator=(Library&& other);
    ~Library();

    /**
     * Check if a library is opened.
     */
    bool IsOpened() const;

    /**
     * Open a dynamic library.
     * @param  path File path.
     * @return true on success.
     */
    bool Open(const StringView path);

    /**
     * Close opened library. All returned function pointers will be invalid after this operation.
     */
    void Close();

    /**
     * Get symbol address by name.
     * @param name Symbol (function or variable) name.
     * @param result Variable to load symbol into.
     * @return False if symbol does not exist or the library is not opened.
     */
    template <typename T>
    bool GetSymbol(const String& name, T& result)
    {
        // workaround: simple casting from data pointer to function pointer is not legal in C++
        *(reinterpret_cast<void**>(&result)) = GetSymbol(name);

        if (!result)
            return false;

        return true;
    }
};

} // namespace Common
} // namespace NFE
