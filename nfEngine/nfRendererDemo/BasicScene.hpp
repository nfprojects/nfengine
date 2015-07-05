/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Declarations of Scene functions
 *
 * The subscenes described in this file are meant to help in new Renderer backend creation process.
 * In theory, each subscene should add a small amount of Renderers Interfaces (ideally, only one),
 * which will represent specific Renderers features. The subscenes should rely on each other - more
 * complex subscene will add new Interface initialization to simplier scenes. This way it can be
 * easily tracked which features are yet to be implemented in a newly developed Renderer backend.
 */

#pragma once

#include "Scene.hpp"

class BasicScene: public Scene
{
    // Helper method, to release all resources which change between subscenes
    void ReleaseSubsceneResources();

public:
    ~BasicScene();

    bool Init(NFE::Renderer::IDevice* rendererDevice, void* winHandle);
    bool SwitchSubScene(size_t subScene);
    size_t GetAvailableSubSceneCount();
    void Draw();
    void Release();
};
