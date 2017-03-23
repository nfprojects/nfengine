/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Backend selector definitions
 */

#include "PCH.hpp"
#include "Backends.hpp"

int gDebugLevel;
int gPreferedCardId;
String gBackend;
String gShaderPathPrefix;
String gShaderPathExt;

const String D3D11_BACKEND("nfRendererD3D11");
const String D3D12_BACKEND("nfRendererD3D12");
const String OGL4_BACKEND("nfRendererOGL4");
const String VK_BACKEND("nfRendererVk");

const String HLSL5_SHADER_PATH_PREFIX("nfEngine/nfRendererTest/Shaders/HLSL5/");
const String HLSL5_SHADER_EXTENSION(".hlsl");

const String GLSL_SHADER_PATH_PREFIX("nfEngine/nfRendererTest/Shaders/GLSL/");
const String GLSL_SHADER_EXTENSION(".glsl");

std::vector<String> GetDefaultBackend()
{
#ifdef WIN32
    return { D3D11_BACKEND, HLSL5_SHADER_PATH_PREFIX, HLSL5_SHADER_EXTENSION };
#elif defined(__linux__) | defined(__LINUX__)
    return { VK_BACKEND, GLSL_SHADER_PATH_PREFIX, GLSL_SHADER_EXTENSION };
#else
#error "Target platform not supported!"
#endif
}
