/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Definitions of common constants for nfRendererDemo
 */

#include "PCH.hpp"

#include "Common.hpp"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

std::string gShaderPathPrefix;
std::string gShaderPathExt;

const std::string D3D11_BACKEND("nfRendererD3D11");
const std::string D3D11_SHADER_PATH_PREFIX("nfEngine/nfRendererDemo/Shaders/D3D11/");
const std::string D3D11_SHADER_EXTENSION(".hlsl");

const std::string OGL4_BACKEND("nfRendererOGL4");
const std::string GLSL_SHADER_PATH_PREFIX("nfEngine/nfRendererDemo/Shaders/GLSL/");
const std::string GLSL_SHADER_EXTENSION(".glsl");

std::vector<std::string> GetDefaultBackend()
{
#ifdef WIN32
    return { D3D11_BACKEND, D3D11_SHADER_PATH_PREFIX, D3D11_SHADER_EXTENSION };
#elif defined(__linux__) | defined(__LINUX__)
    return { OGL4_BACKEND, GLSL_SHADER_PATH_PREFIX, GLSL_SHADER_EXTENSION };
#else
#error "Target platform not supported!"
#endif
}
