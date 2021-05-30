/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Scene base class definition
 */

#include "../PCH.hpp"
#include "Scene.hpp"
#include "Engine/Common/nfCommon.hpp"
#include "Engine/Common/Utils/LanguageUtils.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/Reflection/ReflectionClassDefine.hpp"


using namespace NFE;
using namespace Renderer;

NFE_DEFINE_POLYMORPHIC_CLASS(Scene)
NFE_END_DEFINE_CLASS()

Scene::Scene(const Common::StringView name)
    : mCurrentSubScene(UINT32_MAX)
    , mHighestAvailableSubScene(UINT32_MAX)
    , mName(name)
    , mRendererDevice(nullptr)
{
}

Scene::~Scene()
{
    // destroy queues first so all the commands will flush
    mGraphicsQueue.Reset();
    mCopyQueue.Reset();
}

const Common::StringView Scene::GetSceneName() const
{
    return mName;
}

uint32 Scene::GetCurrentSubSceneNumber() const
{
    return mCurrentSubScene;
}

uint32 Scene::GetAvailableSubSceneCount() const
{
    return mHighestAvailableSubScene;
}

uint32 Scene::GetSubsceneCount() const
{
    return mSubScenes.Size();
}

const Common::StringView Scene::GetCurrentSubSceneName() const
{
    if (mCurrentSubScene > mHighestAvailableSubScene)
        return Common::StringView();

    return mSubScenes[mCurrentSubScene].name;
}

bool Scene::OnSwitchSubscene()
{
    return true;
}

void Scene::RegisterSubScene(SubSceneInitializer initializer, const NFE::Common::StringView name)
{
    SubSceneDefinition def;
    def.initializer = initializer;
    def.name = name;
    mSubScenes.PushBack(def);
}

void Scene::ReleaseSubsceneResources()
{
}

bool Scene::Init(IDevice* rendererDevice, const CommandQueuePtr& graphicsQueue, const CommandQueuePtr& copyQueue, void* winHandle, int32 subscene)
{
    NFE_ASSERT(rendererDevice, "Invalid renderer device");
    NFE_ASSERT(graphicsQueue, "Invalid graphics queue");
    NFE_ASSERT(copyQueue, "Invalid copy queue");

    mRendererDevice = rendererDevice;
    mGraphicsQueue = graphicsQueue;
    mCopyQueue = copyQueue;
    mCommandBuffer = mRendererDevice->CreateCommandRecorder();

    // find suitable back buffer format
    const Format preferredFormats[] =
    {
        // Higher quality formats are temporarily disabled; they cause errors in D3D12 renderer
        // Format::R16G16B16A16_U_Norm,
        // Format::R10G10B10A2_U_Norm,
        Format::R8G8B8A8_U_Norm,
        Format::B8G8R8A8_U_Norm,
        Format::R8G8B8A8_U_Norm_sRGB,
        Format::B8G8R8A8_U_Norm_sRGB,
    };

    mBackbufferFormat = Format::Unknown;
    for (size_t i = 0; i < ArraySize(preferredFormats); ++i)
    {
        if (rendererDevice->IsBackbufferFormatSupported(preferredFormats[i]))
        {
            mBackbufferFormat = preferredFormats[i];
            break;
        }
    }

    if (!OnInit(winHandle))
        return false;

    if (subscene < 0)
    {
        // Basic stuff initialized, try to find the highest subscene possible
        mCurrentSubScene = UINT32_MAX;
        for (mHighestAvailableSubScene = mSubScenes.Size() - 1u; ; mHighestAvailableSubScene--)
        {
            if (SwitchSubscene(mHighestAvailableSubScene))
                break; // the scene initialized successfully

            if (mHighestAvailableSubScene == 0)
            {
                ReleaseSubsceneResources();
                mHighestAvailableSubScene = UINT32_MAX;
                return false; // we hit the end of our scenes vector, no scene successfully initialized
            }
        }

        mCurrentSubScene = mHighestAvailableSubScene;
    }
    else
    {
        // User requested a specific subscene - try to load it and leave if it fails
        if (!SwitchSubscene(subscene))
        {
            NFE_LOG_ERROR("Failed to switch to requested subscene %u", subscene);
            return false;
        }
    }

    return true;
}

bool Scene::SwitchSubscene(uint32 subScene)
{
    if (subScene == mCurrentSubScene)
        return true;

    mCurrentSubScene = UINT32_MAX;
    ReleaseSubsceneResources();

    if (subScene > mHighestAvailableSubScene)
        return false;
    if (!mSubScenes[subScene].initializer())
        return false;

    if (!OnSwitchSubscene())
        return false;

    mCurrentSubScene = subScene;
    return true;
}

bool Scene::OnInit(void* winHandle)
{
    TextureDesc texDesc;
    texDesc.width = WINDOW_WIDTH;
    texDesc.height = WINDOW_HEIGHT;
    texDesc.type = TextureType::Texture2D;
    texDesc.format = Format::R8G8B8A8_U_Norm;
    texDesc.usage = TextureUsageFlag::RenderTarget;
    texDesc.mode = ResourceAccessMode::GPUOnly;
    texDesc.debugName = "Scene::mWindowRenderTargetTexture";
    mWindowRenderTargetTexture = mRendererDevice->CreateTexture(texDesc);
    if (!mWindowRenderTargetTexture)
        return false;

    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = WINDOW_WIDTH;
    bbDesc.height = WINDOW_HEIGHT;
    bbDesc.format = mBackbufferFormat;
    bbDesc.windowHandle = winHandle;
    bbDesc.commandQueue = mGraphicsQueue;
    mWindowBackbuffer = mRendererDevice->CreateBackbuffer(bbDesc);
    if (!mWindowBackbuffer)
        return false;

    return true;
}

ShaderPtr Scene::CompileShader(const char* path, ShaderType type, ShaderMacro* macros, uint32 macrosNum)
{
    ShaderDesc desc;
    desc.type = type;
    desc.path = path;
    desc.macros = macros;
    desc.macrosNum = macrosNum;
    return mRendererDevice->CreateShader(desc);
}