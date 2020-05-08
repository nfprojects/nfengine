/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Windows implementation of Library class.
 */

#include "PCH.hpp"
#include "../Library.hpp"
#include "Logger/Logger.hpp"
#include "Common.hpp"


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
    mModule = std::move(other.mModule);
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

    String pathExt = path;
    const String& libExt(".dll");
    if (!path.ToView().EndsWith(libExt))
    {
        pathExt += libExt;
    }

    Utf16String widePath;
    if (!UTF8ToUTF16(pathExt, widePath))
        return false;

    mModule = ::LoadLibrary(widePath.c_str());

    if (mModule == nullptr)
    {
        NFE_LOG_ERROR("Failed to load library '%s': %s", pathExt.Str(), GetLastErrorString().Str());
        return false;
    }

    return true;
}

void Library::Close()
{
    if (mModule != nullptr)
    {
        ::FreeLibrary(mModule);
        mModule = nullptr;
    }
}

void* Library::GetSymbol(const String& name)
{
    if (mModule == nullptr)
        return nullptr;

    FARPROC ptr = ::GetProcAddress(mModule, name.Str());
    if (ptr == nullptr)
    {
        NFE_LOG_ERROR("Failed to get pointer to symbol '%s': %s", name.Str(), GetLastErrorString().Str());
        return nullptr;
    }

    return static_cast<void*>(ptr);
}

} // namespace Common
} // namespace NFE
