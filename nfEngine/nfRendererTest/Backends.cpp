/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Backend selector definitions
 */

#include "PCH.hpp"
#include "Backends.hpp"

std::string gBackend;
std::string gShaderPathPrefix;
std::string gShaderPathExt;

const std::string D3D11_BACKEND("nfRendererD3D11");
const std::string OGL4_BACKEND("nfRendererOGL4");
const std::string VK_BACKEND("nfRendererVk");

const std::string HLSL5_SHADER_PATH_PREFIX("nfEngine/nfRendererTest/Shaders/HLSL5/");
const std::string HLSL5_SHADER_EXTENSION(".hlsl");

const std::string GLSL_SHADER_PATH_PREFIX("nfEngine/nfRendererTest/Shaders/GLSL/");
const std::string GLSL_SHADER_EXTENSION(".glsl");

std::vector<std::string> GetDefaultBackend()
{
#ifdef WIN32
    return { D3D11_BACKEND, HLSL5_SHADER_PATH_PREFIX, HLSL5_SHADER_EXTENSION };
#elif defined(__linux__) | defined(__LINUX__)
    return { VK_BACKEND, GLSL_SHADER_PATH_PREFIX, GLSL_SHADER_EXTENSION };
#else
#error "Target platform not supported!"
#endif
}