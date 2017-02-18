/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Scene base class definition
 */

#pragma once

#include "../Common.hpp"

#include "Renderers/RendererInterface/Device.hpp"

#include <vector>

typedef std::function<bool()> SubSceneInitializer;

struct SubSceneDefinition
{
    SubSceneInitializer initializer;
    std::string name;
};

/**
 * Base class describing a scene
 */
class Scene
{
    size_t mCurrentSubScene;
    size_t mHighestAvailableSubScene;
    std::string mName;  //< scene name
    std::vector<SubSceneDefinition> mSubScenes;

protected:
    // Renderer interfaces
    NFE::Renderer::IDevice* mRendererDevice;
    std::unique_ptr<NFE::Renderer::ICommandRecorder> mCommandBuffer;
    NFE::Renderer::ElementFormat mBackbufferFormat;

    virtual bool OnInit(void* winHandle) = 0;
    virtual bool OnSwitchSubscene();
    void RegisterSubScene(SubSceneInitializer initializer, const std::string& name);

    // Methods common for all scenes:

    NFE::Renderer::IShader* CompileShader(const char* path,
                                          NFE::Renderer::ShaderType type,
                                          NFE::Renderer::ShaderMacro* macros,
                                          size_t macrosNum);

public:
    Scene(const std::string& name);

    /**
     * Virtual destructor for Scene
     *
     * Each derived scene should define the descrutor to clean up.
     */
    virtual ~Scene() {};

    /**
     * Retrieves scene name
     */
    std::string GetSceneName() const;

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
    bool Init(NFE::Renderer::IDevice* rendererDevice, void* winHandle);

    /**
     * Swiches the SubScene in current Scene
     *
     * @param subScene Subscene ID to switch to
     * @return True on successful switch, false if subscene was unable to initialize
     */
    virtual bool SwitchSubscene(size_t subScene);

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
    size_t GetAvailableSubSceneCount() const;

    /**
     * Retrieves current subscene number
     *
     * @return Current subscene number or a negative value when no exists.
     */
    size_t GetCurrentSubSceneNumber() const;

    /**
     * Retrieves current subscene name
     */
    std::string GetCurrentSubSceneName() const;

    /**
     * Drawing call
     *
     * @param dt Time passed since last call
     *
     * @note The method is called every frame in Drawing loop
     * @see DemoWindow::DrawLoop
     */
    virtual void Draw(float dt) = 0;

    /**
     * Releases subScene resources
     */
    virtual void ReleaseSubsceneResources();

    /**
     * Releases scene resources
     */
    virtual void Release() = 0;
};

typedef std::vector<std::unique_ptr<Scene>> SceneArrayType;
