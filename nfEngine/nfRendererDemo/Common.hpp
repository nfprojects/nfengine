/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declarations of common constants for nfRendererDemo
 */

#pragma once

#include <vector>
#include <functional>

typedef std::vector<std::function<bool()>> SubSceneArrayType;

extern const int WINDOW_WIDTH;
extern const int WINDOW_HEIGHT;

extern const std::string D3D11_BACKEND;
extern const std::string D3D11_SHADER_PATH_PREFIX;
extern const std::string D3D11_SHADER_EXTENSION;

extern const std::string OGL4_BACKEND;
extern const std::string OGL4_SHADER_PATH_PREFIX;
extern const std::string OGL4_SHADER_EXTENSION;
