/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OpenGL Master Context singleton definitions for Windows
 */

#include "../MasterContext.hpp"

#include "ContextDataWGL.hpp"

#include "GL/wglext.h"

namespace NFE {
namespace Renderer {


MasterContext::MasterContext()
    : mData(nullptr)
{
}

MasterContext::~MasterContext()
{
    Release();
}

MasterContext& MasterContext::Instance()
{
    static MasterContext instance;
    return instance;
}

bool MasterContext::Init()
{
    ContextDataWGL data;

    // Create a dummy context first, to extract wglCreateContextAttribsARB

    // Function extracted, no need for our dummy context now

    // Create a new context with proper FB settings

    // Acquire OGL extensions
    if (!nfglExtensionsInit())
        return false;

    mData = reinterpret_cast<void*>(new ContextDataWGL(data));

    return true;
}

void MasterContext::Release()
{
    if (mData)
    {
        // free the context

        // free unused data
        delete data;
        mData = nullptr;
    }
}

const void* MasterContext::GetData()
{
    return mData;
}

} // namespace Renderer
} // namespace NFE
