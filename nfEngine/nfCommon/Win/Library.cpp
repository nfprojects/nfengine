/**
 * @file   Library.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Windows implementation of Library class.
 */

#include "../stdafx.hpp"
#include "../Library.hpp"
#include "../Logger.hpp"
#include "Common.hpp"

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

    std::wstring widePath;
    if (!UTF8ToUTF16(path, widePath))
        return false;

    mModule = ::LoadLibrary(widePath.c_str());

    if (mModule == NULL)
    {
        LOG_ERROR("Failed to load library '%s': %s", path.c_str(), GetLastErrorString().c_str());
        return false;
    }

    return true;
}

void Library::Close()
{
    if (mModule == NULL)
    {
        ::FreeLibrary(mModule);
        mModule = NULL;
    }
}

void* Library::GetSymbol(const std::string& name)
{
    if (mModule == NULL)
        return nullptr;

    FARPROC ptr = ::GetProcAddress(mModule, name.c_str());
    if (ptr == NULL)
    {
        LOG_ERROR("Failed to get pointer to symbol '%s': %s", name.c_str(),
                  GetLastErrorString().c_str());
        return nullptr;
    }

    return static_cast<void*>(ptr);
}

} // namespace Common
} // namespace NFE
