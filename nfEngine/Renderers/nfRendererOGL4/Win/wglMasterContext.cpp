/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OpenGL Master Context singleton definitions for Windows
 */

#include "../PCH.hpp"

#include "../MasterContext.hpp"
#include "wglContextData.hpp"

//#include "GL/wglext.h"

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
    wglContextData data;

    // Create a dummy context first, to extract wglCreateContextAttribsARB

    // Function extracted, no need for our dummy context now

    // Create a new context with proper FB settings

    // Acquire OGL extensions
    // TODO for now this produces loads of errors due to no context.
    if (!nfglExtensionsInit())
        return false;

    mData = reinterpret_cast<void*>(new wglContextData(data));

    return true;
}

void MasterContext::Release()
{
    if (mData)
    {
        wglContextData* data = reinterpret_cast<wglContextData*>(mData);
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
