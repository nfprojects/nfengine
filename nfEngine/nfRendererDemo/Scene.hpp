/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Scene base class definition
 */

#pragma once

#include "Common.hpp"

#include "../Renderers/RendererInterface/Device.hpp"

#include <vector>

/**
 * Base class describing a scene
 */
class Scene
{
public:
    /**
     * Virtual destructor for Scene
     *
     * Each derived scene should define the descrutor to clean up.
     */
    virtual ~Scene() {};

    /**
     * Initializes the scene
     *
     * @param rendererDevice Pointer to valid NFE::Renderer::IDevice interface.
     * @param winHandle      Valid window handle, used by NFE::Renderer::Backbuffer during init.
     * @return True on success, false otherwise.
     *
     * The method provides rendererDevice for further devices initialization, and winHandle in case
     * Backbuffer must be initialized.
     */
    virtual bool Init(NFE::Renderer::IDevice* rendererDevice, void* winHandle) = 0;

    /**
     * Swiches the SubScene in current Scene
     *
     * @param subScene Subscene ID to switch to
     * @return True on successful switch, false if subscene was unable to initialize
     *
     * @note
     * The check for correct Subscene ID is performed in DemoWindow class. Therefore, it is safe to
     * assume, that @subScene is correct ID and just perform the switch.
     */
    virtual bool SwitchSubScene(size_t subScene) = 0;

    /**
     * Retrieves subscene count
     *
     * @return Available subscene count in current scene
     *
     * @note
     * This method is used by DemoWindow to determine what is the highest available subscene in
     * currently loaded scene. If some subscenes cannot be loaded (ex. because some Renderer
     * backend features are not yet implemented), it is recommended to detect such situation and
     * set this value to lower than all subscene count.
     */
    virtual size_t GetAvailableSubSceneCount() = 0;

    /**
     * Drawing call
     *
     * The method is called every frame in Drawing loop
     *
     * @see DemoWindow::DrawLoop
     */
    virtual void Draw() = 0;

    /**
     * Releases scene resources
     */
    virtual void Release() = 0;
};

typedef std::vector<std::unique_ptr<Scene>> SceneArrayType;
