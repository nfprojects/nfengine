/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Backend selector declarations
 */

#pragma once

/// renderer configuration
extern int gDebugLevel;
extern int gPreferedCardId;
extern String gBackend;
extern String gShaderPathPrefix;
extern String gShaderPathExt;

/// Renderer backends
extern const String D3D11_BACKEND;
extern const String D3D12_BACKEND;
extern const String OGL4_BACKEND;
extern const String VK_BACKEND;

extern const String HLSL5_SHADER_PATH_PREFIX;
extern const String HLSL5_SHADER_EXTENSION;

extern const String GLSL_SHADER_PATH_PREFIX;
extern const String GLSL_SHADER_EXTENSION;

/**
 * Returns a default backend, according to current system.
 *
 * @return Three-element vector with default backend strings
 *
 * @note First element is used library, second is shader path prefix, third is shader extension
 */
std::vector<String> GetDefaultBackend();
