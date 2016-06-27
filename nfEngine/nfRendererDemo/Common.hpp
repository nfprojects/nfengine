/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declarations of common constants for nfRendererDemo
 */

#pragma once

#include <vector>
#include <functional>

extern const int WINDOW_WIDTH;
extern const int WINDOW_HEIGHT;

/// Shaders path prefix and extension, used to load appropriate shader source file
extern std::string gShaderPathPrefix;
extern std::string gShaderPathExt;

extern const std::string D3D11_BACKEND;
extern const std::string D3D11_SHADER_PATH_PREFIX;
extern const std::string D3D11_SHADER_EXTENSION;

extern const std::string OGL4_BACKEND;
extern const std::string GLSL_SHADER_PATH_PREFIX;
extern const std::string GLSL_SHADER_EXTENSION;

extern const std::string D3D12_BACKEND;
extern const std::string D3D12_SHADER_PATH_PREFIX;
extern const std::string D3D12_SHADER_EXTENSION;

/**
 * Returns a default backend, according to current system.
 *
 * @return Three-element vector with default backend strings
 *
 * @note First element is used library, second is shader path prefix, third is shader extension
 */
std::vector<std::string> GetDefaultBackend();
