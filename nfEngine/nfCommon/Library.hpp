/**
 * @file
 * @author Mkkulagowski (mkkulagowski(at)gmail.com)
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
    void* mModule;
#else
#error "Target system not supported!"
#endif
    //
    void* GetSymbol(const std::string& name);

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
     * Open a dynamic library.
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
     * @oaram result Variable to load symbol into.
     * @return False if symbol does not exist or the library is not opened.
     */
    template <typename T>
    bool GetSymbol(const std::string& name, T& result)
    {
        result = (T) GetSymbol(name);
        if (!result)
        {
            std::cout << "result is nullptr: " << (result == nullptr) << " or NULL: " << (result == NULL) << std::endl;
            return false;
        }

        return true;
    }
};

} // namespace Common
} // namespace NFE
