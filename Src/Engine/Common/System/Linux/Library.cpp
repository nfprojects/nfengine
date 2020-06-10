/**
 * @file
 * @author Mkkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Linux implementation of Library class.
 */

#include "PCH.hpp"
#include "../Library.hpp"
#include "Logger/Logger.hpp"
#include "Containers/String.hpp"

#include <dlfcn.h>


namespace NFE {
namespace Common {

Library::Library()
    : mModule(nullptr)
{
}

Library::Library(const StringView& path)
    : Library()
{
    Open(path);
}

Library::Library(Library&& other)
    : Library()
{
    std::swap(mModule, other.mModule);
}

Library& Library::operator=(Library&& other)
{
    if (this != &other)
    {
        Close();
        std::swap(mModule, other.mModule);
    }
    return *this;
}

Library::~Library()
{
    Close();
}

bool Library::IsOpened() const
{
    return mModule != nullptr;
}

bool Library::Open(const StringView& path)
{
    Close();

    String pathExt = "lib" + path;
    const StringView libExt(".so");
    if (!path.EndsWith(libExt))
    {
        pathExt += libExt;
    }

    mModule = dlopen(pathExt.Str(), RTLD_LAZY);

    if (mModule == nullptr)
    {
        NFE_LOG_ERROR("Failed to load library '%s': %s", pathExt.Str(), dlerror());
        return false;
    }

    return true;
}

void Library::Close()
{
    if (mModule != nullptr)
    {
        if (dlclose(mModule))
            NFE_LOG_ERROR("Failed to close library: %s", dlerror());
        mModule = nullptr;
    }
}

void* Library::GetSymbol(const StringView& name)
{
    if (mModule == nullptr)
        return nullptr;

    const StringViewToCStringHelper nameCString(name);

    // it is recommended to clear dlerror first, because nullptr value returned CAN be valid
    dlerror();
    void* ptr = dlsym(mModule, nameCString);
    char* errorMsg = dlerror();
    if (errorMsg != nullptr)
    {
        NFE_LOG_ERROR("Failed to get pointer to symbol '%s': %s", nameCString, errorMsg);
        return nullptr;
    }

    return ptr;
}

} // namespace Common
} // namespace NFE
