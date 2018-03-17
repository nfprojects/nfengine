/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Backend selector definitions
 */

#include "PCH.hpp"
#include "Backends.hpp"

int gDebugLevel;
int gPreferedCardId;
NFE::Common::String gBackend;
NFE::Common::String gShaderPathPrefix;
NFE::Common::String gShaderPathExt;

const NFE::Common::String D3D11_BACKEND("nfRendererD3D11");
const NFE::Common::String D3D12_BACKEND("nfRendererD3D12");
const NFE::Common::String VK_BACKEND("nfRendererVk");

const NFE::Common::String HLSL5_SHADER_PATH_PREFIX("nfEngine/nfRendererTest/Shaders/HLSL5/");
const NFE::Common::String HLSL5_SHADER_EXTENSION(".hlsl");

const NFE::Common::String GLSL_SHADER_PATH_PREFIX("nfEngine/nfRendererTest/Shaders/GLSL/");
const NFE::Common::String GLSL_SHADER_EXTENSION(".glsl");

NFE::Common::DynArray<NFE::Common::String> GetDefaultBackend()
{
#ifdef WIN32
    return { D3D11_BACKEND, HLSL5_SHADER_PATH_PREFIX, HLSL5_SHADER_EXTENSION };
#elif defined(__linux__) | defined(__LINUX__)
    return { VK_BACKEND, GLSL_SHADER_PATH_PREFIX, GLSL_SHADER_EXTENSION };
#else
#error "Target platform not supported!"
#endif
}
