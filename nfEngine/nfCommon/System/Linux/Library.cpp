/**
 * @file
 * @author Mkkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Linux implementation of Library class.
 */

#include "PCH.hpp"
#include "../Library.hpp"
#include "Logger/Logger.hpp"

#include <dlfcn.h>


namespace NFE {
namespace Common {

Library::Library()
    : mModule(nullptr)
{
}

Library::Library(const String& path)
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

bool Library::Open(const String& path)
{
    Close();

    String pathExt = "lib" + path;
    const StringView libExt(".so");
    if (!path.ToView().EndsWith(libExt))
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

void* Library::GetSymbol(const String& name)
{
    if (mModule == nullptr)
        return nullptr;

    // it is recommended to clear dlerror first, because nullptr value returned CAN be valid
    dlerror();
    void* ptr = dlsym(mModule, name.Str());
    char* errorMsg = dlerror();
    if (errorMsg != nullptr)
    {
        NFE_LOG_ERROR("Failed to get pointer to symbol '%s': %s", name.Str(), errorMsg);
        return nullptr;
    }

    return ptr;
}

} // namespace Common
} // namespace NFE
