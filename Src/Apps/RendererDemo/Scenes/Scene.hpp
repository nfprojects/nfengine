/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Scene base class definition
 */

#pragma once

#include "../Common.hpp"

#include "Engine/Renderers/RendererCommon/Device.hpp"
#include "Engine/Common/Reflection/ReflectionClassDeclare.hpp"
#include "Engine/Common/Containers/String.hpp"

typedef std::function<bool()> SubSceneInitializer;

struct SubSceneDefinition
{
    SubSceneInitializer initializer;
    NFE::Common::StringView name;
};

/**
 * Base class describing a scene
 */
class Scene : public NFE::IObject
{
    NFE_DECLARE_POLYMORPHIC_CLASS(Scene)

    NFE::uint32 mCurrentSubScene;
    NFE::uint32 mHighestAvailableSubScene;
    NFE::Common::StringView mName;
    NFE::Common::DynArray<SubSceneDefinition> mSubScenes;

protected:
    // Renderer interfaces
    NFE::Renderer::IDevice* mRendererDevice;
    NFE::Renderer::CommandQueuePtr mGraphicsQueue;
    NFE::Renderer::CommandQueuePtr mCopyQueue;
    NFE::Renderer::CommandRecorderPtr mCommandBuffer;
    NFE::Renderer::Format mBackbufferFormat;

    NFE::Renderer::BackbufferPtr mWindowBackbuffer;
    NFE::Renderer::TexturePtr mWindowRenderTargetTexture;

    virtual bool OnInit(void* winHandle);
    virtual bool OnSwitchSubscene();
    void RegisterSubScene(SubSceneInitializer initializer, const NFE::Common::StringView name);

    // Methods common for all scenes:
    NFE::Renderer::ShaderPtr CompileShader(const char* path, NFE::Renderer::ShaderType type, NFE::Renderer::ShaderMacro* macros, NFE::uint32 macrosNum);

public:
    Scene(const NFE::Common::StringView name);

    /**
     * Virtual destructor for Scene
     *
     * Each derived scene should define the destructor to clean up.
     */
    virtual ~Scene();

    /**
     * Retrieves scene name
     */
    const NFE::Common::StringView GetSceneName() const;

    /**
     * Initializes the scene
     *
     * @param rendererDevice Pointer to valid NFE::Renderer::IDevice interface.
     * @param graphicsQueue  Graphics-capable Queue to perform rendering operations on
     * @param copyQueue      Copy-capable Queue for transferring data to/from GPU
     * @param winHandle      Valid window handle, used by NFE::Renderer::Backbuffer during init.
     * @param subscene       Subscene ID to
     * @return True on success, false otherwise.
     *
     * The method provides rendererDevice for further devices initialization, and winHandle in case
     * Backbuffer must be initialized.
     */
    bool Init(NFE::Renderer::IDevice* rendererDevice, const NFE::Renderer::CommandQueuePtr& graphicsQueue,
              const NFE::Renderer::CommandQueuePtr& copyQueue, void* winHandle, NFE::int32 subscene = -1);

    /**
     * Switches the SubScene in current Scene
     *
     * @param subScene Subscene ID to switch to
     * @return True on successful switch, false if subscene was unable to initialize
     */
    virtual bool SwitchSubscene(NFE::uint32 subScene);

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
    NFE::uint32 GetAvailableSubSceneCount() const;

    /**
     * Retrieves total amount of subscenes
     *
     * @return Amount of all available subscenes
     *
     * @note Compared to GetAvailableSubSceneCount() this call returns how many Subscenes are
     * actually registered. This does not mean that all are working/available, but helps to
     * determine how many Subscenes given Scene has before calling Init() method.
     */
    NFE::uint32 GetSubsceneCount() const;

    /**
     * Retrieves current subscene number
     *
     * @return Current subscene number or a negative value when no exists.
     */
    NFE::uint32 GetCurrentSubSceneNumber() const;

    /**
     * Retrieves current subscene name
     */
    const NFE::Common::StringView GetCurrentSubSceneName() const;

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

using SceneArrayType = NFE::Common::DynArray<NFE::Common::UniquePtr<Scene>>;
