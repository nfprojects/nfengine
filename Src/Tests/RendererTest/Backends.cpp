/**
 * @file
 * @author Lookey (costyrra.xl@gmail.com)
 * @brief  Backend selector definitions
 */

#include "PCH.hpp"
#include "Backends.hpp"

int gDebugLevel;
int gPreferedCardId;
NFE::Common::String gBackend;
NFE::Common::String gShaderPathPrefix;
NFE::Common::String gShaderPathExt;

const NFE::Common::String D3D12_BACKEND("RendererD3D12");
const NFE::Common::String VK_BACKEND("RendererVk");

const NFE::Common::String HLSL5_SHADER_PATH_PREFIX("Src/Tests/RendererTest/Shaders/HLSL5/");
const NFE::Common::String HLSL5_SHADER_EXTENSION(".hlsl");

const NFE::Common::String GLSL_SHADER_PATH_PREFIX("Src/Tests/RendererTest/Shaders/GLSL/");
const NFE::Common::String GLSL_SHADER_EXTENSION(".glsl");

NFE::Common::DynArray<NFE::Common::String> GetDefaultBackend()
{
#ifdef NFE_PLATFORM_WINDOWS
    return { D3D12_BACKEND, HLSL5_SHADER_PATH_PREFIX, HLSL5_SHADER_EXTENSION };
#elif defined(NFE_PLATFORM_LINUX)
    return { VK_BACKEND, GLSL_SHADER_PATH_PREFIX, GLSL_SHADER_EXTENSION };
#else
#error Invalid platform
#endif
}
