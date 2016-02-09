/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OpenGL Master Context singleton definitions for Windows
 */

#include "../PCH.hpp"

#include "../MasterContext.hpp"

namespace NFE {
namespace Renderer {


MasterContext::MasterContext()
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
    // Create a dummy context first, to extract wglCreateContextAttribsARB

    // Function extracted, no need for our dummy context now

    // Create a new context with proper FB settings

    // Acquire OGL extensions
    // TODO for now this produces loads of errors due to no context.
    if (!nfglExtensionsInit())
        return false;

    return true;
}

void MasterContext::Release()
{

}

} // namespace Renderer
} // namespace NFE
