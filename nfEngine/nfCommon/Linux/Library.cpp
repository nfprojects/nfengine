/**
 * @file
 * @author Mkkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Linux implementation of Library class.
 */

#include "../PCH.hpp"
#include "../Library.hpp"
#include "../Logger.hpp"
#include "Common.hpp"
#include <dlfcn.h>

namespace NFE {
namespace Common {

Library::Library()
{
    mModule = NULL;
}

Library::Library(const std::string& path)
{
    Open(path);
}

Library::Library(Library&& other)
{
    Close();
    mModule = other.mModule;
    other.mModule = NULL;
}

Library::~Library()
{
    Close();
}

bool Library::IsOpened() const
{
    return mModule != NULL;
}

bool Library::Open(const std::string& path)
{
    Close();

    std::string pathExt(path);
    std::string libExt = ".so";
    if (libExt.compare(pathExt.substr(pathExt.size() - libExt.size())) != 0)
        pathExt.append(libExt);

    mModule = dlopen(pathExt.c_str(), RTLD_LAZY | RTLD_GLOBAL);

    if (mModule == NULL)
    {
        LOG_ERROR("Failed to load library '%s': %s", pathExt.c_str(), dlerror());
        return false;
    }

    return true;
}

void Library::Close()
{
    if (mModule != NULL)
    {
        if (dlclose(mModule))
            LOG_ERROR("Failed to close library: %s", dlerror());
        mModule = NULL;
    }
}

void* Library::GetSymbol(const std::string& name)
{
    if (mModule == NULL)
        return nullptr;

    // it is recommended to clear dlerror first, because NULL value returned CAN be valid
    dlerror();
    void* ptr = dlsym(mModule, name.c_str());
    char* errorMsg = dlerror();
    if (errorMsg != NULL)
    {
        LOG_ERROR("Failed to get pointer to symbol '%s': %s", name.c_str(), errorMsg);
        return nullptr;
    }

    return ptr;
}

} // namespace Common
} // namespace NFE
