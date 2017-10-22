/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Definitions of common constants for nfRendererDemo
 */

#include "PCH.hpp"

#include "Common.hpp"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

NFE::Common::String gShaderPathPrefix;
NFE::Common::String gShaderPathExt;

const NFE::Common::String D3D11_BACKEND("nfRendererD3D11");
const NFE::Common::String D3D12_BACKEND("nfRendererD3D12");
const NFE::Common::String OGL4_BACKEND("nfRendererOGL4");
const NFE::Common::String VK_BACKEND("nfRendererVk");

const NFE::Common::String HLSL5_SHADER_PATH_PREFIX("nfEngine/nfRendererDemo/Shaders/HLSL5/");
const NFE::Common::String HLSL5_SHADER_EXTENSION(".hlsl");

const NFE::Common::String GLSL_SHADER_PATH_PREFIX("nfEngine/nfRendererDemo/Shaders/GLSL/");
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
