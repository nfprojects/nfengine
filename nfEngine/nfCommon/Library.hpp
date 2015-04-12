/**
 * @file   Library.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Library class declaration.
 */

#pragma once

#include "nfCommon.hpp"

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
    // TODO
#else
#error "Target system not supported!"
#endif

public:
    Library();
    Library(const std::string& path);
    Library(Library&& other);
    ~Library();

    /**
     * Check if a library is opened.
     */
    bool IsOpened() const;

    /**
     * Open a dyniamic library.
     * @param  path File path.
     * @return true on success.
     */
    bool Open(const std::string& path);

    /**
     * Close opened library. All returned function pointers will be invalid after this operation.
     */
    void Close();

    /**
     * Get symbol address by name.
     * @param name Symbol (function or variable) name.
     * @return Null pointer if symbol does not exist or the library is not opened.
     */
    void* GetSymbol(const std::string& name);
};

} // namespace Common
} // namespace NFE
