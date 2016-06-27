/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Backend selector declarations
 */

#pragma once

/// Shaders path prefix and extension, used to load appropriate shader source file
extern std::string gBackend;
extern std::string gShaderPathPrefix;
extern std::string gShaderPathExt;

/// Renderer backends
extern const std::string D3D11_BACKEND;
extern const std::string OGL4_BACKEND;
extern const std::string VK_BACKEND;

extern const std::string HLSL5_SHADER_PATH_PREFIX;
extern const std::string HLSL5_SHADER_EXTENSION;

extern const std::string GLSL_SHADER_PATH_PREFIX;
extern const std::string GLSL_SHADER_EXTENSION;

/**
 * Returns a default backend, according to current system.
 *
 * @return Three-element vector with default backend strings
 *
 * @note First element is used library, second is shader path prefix, third is shader extension
 */
std::vector<std::string> GetDefaultBackend();
