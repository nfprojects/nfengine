/**
 * @file
 * @author Mkkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Linux implementation of Library class.
 */

#include "../PCH.hpp"
#include "../Library.hpp"
#include "../Logger.hpp"

#include <dlfcn.h>

namespace NFE {
namespace Common {

Library::Library()
    : mModule(nullptr)
{
}

Library::Library(const std::string& path)
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

bool Library::Open(const std::string& path)
{
    Close();

    std::string pathExt = "lib";
    pathExt += path;
    std::string libExt = ".so";
    if (libExt.compare(pathExt.substr(pathExt.size() - libExt.size())) != 0)
        pathExt.append(libExt);

    mModule = dlopen(pathExt.c_str(), RTLD_LAZY);

    if (mModule == nullptr)
    {
        LOG_ERROR("Failed to load library '%s': %s", pathExt.c_str(), dlerror());
        return false;
    }

    return true;
}

void Library::Close()
{
    if (mModule != nullptr)
    {
        if (dlclose(mModule))
            LOG_ERROR("Failed to close library: %s", dlerror());
        mModule = nullptr;
    }
}

void* Library::GetSymbol(const std::string& name)
{
    if (mModule == nullptr)
        return nullptr;

    // it is recommended to clear dlerror first, because nullptr value returned CAN be valid
    dlerror();
    void* ptr = dlsym(mModule, name.c_str());
    char* errorMsg = dlerror();
    if (errorMsg != nullptr)
    {
        LOG_ERROR("Failed to get pointer to symbol '%s': %s", name.c_str(), errorMsg);
        return nullptr;
    }

    return ptr;
}

} // namespace Common
} // namespace NFE
