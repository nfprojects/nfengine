/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declarations of common constants for nfRendererDemo
 */

#pragma once

#include <vector>
#include <functional>

typedef std::vector<std::function<bool()>> SubSceneArrayType;
typedef std::vector<SubSceneArrayType> SceneArrayType;

extern const int WINDOW_WIDTH;
extern const int WINDOW_HEIGHT;
