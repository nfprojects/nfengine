/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Source file with scene definitions
 *
 * The scenes described in this file are meant to help in new Renderer backend creation process.
 * In theory, each scene should add a small amount of Renderers Interfaces (ideally, only one),
 * which will represent specific Renderers features. The scenes should rely on each other - more
 * complex scene will add new Interface initialization to simplier scenes. This way it can be
 * easily tracked which features are yet to be implemented in a newly developed Renderer backend.
 *
 * To shorten the code used to create scenes, all Renderer-related code was moved to RendererUtils
 * module. Ideally Scenes should have no idea that such thing as nfRenderer exists and it should
 * only use RendererUtils functions with different arguments, according to different scenes.
 */

#include "PCH.hpp"

#include "BasicScene.hpp"
#include "Common.hpp"
#include "RendererUtils.hpp"

#include <vector>
#include <functional>

/// Scene declarations
bool CreateSubSceneEmpty();
bool CreateSubSceneVertexBuffer();
bool CreateSubSceneIndexBuffer();
bool CreateSubSceneConstantBuffer();
bool CreateSubSceneTexture();

// SubScene container
SubSceneArrayType gSubScenes =
{
    CreateSubSceneEmpty,
    CreateSubSceneVertexBuffer,
    CreateSubSceneIndexBuffer,
    CreateSubSceneConstantBuffer,
    CreateSubSceneTexture,
};

/// Utility APIs, used in Main.cpp
bool InitSubScene(size_t sceneId)
{
    return gSubScenes[sceneId]();
}

size_t GetSubSceneCount()
{
    return gSubScenes.size();
}


/// Subscenes

// Basic initialization, additionally to RT & BackBuffer shaders are compiled
// Empty window should be visible
bool CreateSubSceneEmpty()
{
    return CreateShaderProgram(false, false);
}

// Adds vertex buffer creation
// Two colored triangles should be visible
bool CreateSubSceneVertexBuffer()
{
    if (!CreateShaderProgram(false, false))
        return false;

    return CreateVertexBuffer(false);
}

// Adds IndexBuffer creation
// A colored triangle and a colored square should be visible
bool CreateSubSceneIndexBuffer()
{
    if (!CreateShaderProgram(false, false))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    return CreateIndexBuffer();
}

// Adds constant buffers
// The triangle and the square should rotate
bool CreateSubSceneConstantBuffer()
{
    if (!CreateShaderProgram(true, false))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    if (!CreateIndexBuffer())
        return false;

    return CreateConstantBuffer();
}

// Add texture support
// The triangle should be rendered checked
bool CreateSubSceneTexture()
{
    if (!CreateShaderProgram(true, true))
        return false;

    if (!CreateVertexBuffer(true))
        return false;

    if (!CreateIndexBuffer())
        return false;

    if (!CreateConstantBuffer())
        return false;

    return CreateTexture();
}
