/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declarations of common constants for nfRendererDemo
 */

#pragma once

#include "Engine/Common/Containers/String.hpp"
#include "Engine/Common/Containers/DynArray.hpp"


extern const int WINDOW_WIDTH;
extern const int WINDOW_HEIGHT;

/// Shaders path prefix and extension, used to load appropriate shader source file
extern NFE::Common::String gShaderPathPrefix;
extern NFE::Common::String gShaderPathExt;

/// Renderer backends
extern const NFE::Common::String D3D12_BACKEND;
extern const NFE::Common::String VK_BACKEND;

extern const NFE::Common::String HLSL5_SHADER_PATH_PREFIX;
extern const NFE::Common::String HLSL5_SHADER_EXTENSION;

extern const NFE::Common::String GLSL_SHADER_PATH_PREFIX;
extern const NFE::Common::String GLSL_SHADER_EXTENSION;

/**
 * Returns a default backend, according to current system.
 *
 * @return Three-element vector with default backend strings
 *
 * @note First element is used library, second is shader path prefix, third is shader extension
 */
NFE::Common::DynArray<NFE::Common::String> GetDefaultBackend();

