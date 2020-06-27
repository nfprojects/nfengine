/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Scene base class definition
 */

#include "../PCH.hpp"
#include "Scene.hpp"
#include "Engine/Common/nfCommon.hpp"
#include "Engine/Common/Utils/LanguageUtils.hpp"


using namespace NFE;
using namespace Renderer;

Scene::Scene(const std::string& name)
    : mCurrentSubScene(SIZE_MAX)
    , mHighestAvailableSubScene(SIZE_MAX)
    , mName(name)
    , mRendererDevice(nullptr)
{
}

std::string Scene::GetSceneName() const
{
    return mName;
}

size_t Scene::GetCurrentSubSceneNumber() const
{
    return mCurrentSubScene;
}

size_t Scene::GetAvailableSubSceneCount() const
{
    return mHighestAvailableSubScene;
}

std::string Scene::GetCurrentSubSceneName() const
{
    if (mCurrentSubScene > mHighestAvailableSubScene)
        return std::string();

    return mSubScenes[mCurrentSubScene].name;
}

bool Scene::OnSwitchSubscene()
{
    return true;
}

void Scene::RegisterSubScene(SubSceneInitializer initializer, const std::string& name)
{
    SubSceneDefinition def;
    def.initializer = initializer;
    def.name = name;
    mSubScenes.push_back(def);
}

void Scene::ReleaseSubsceneResources()
{
    // HACK
    // TODO remove (requires fix in D3D12 renderer)
    if (mRendererDevice)
    {
        mRendererDevice->WaitForGPU();
    }
}

bool Scene::Init(IDevice* rendererDevice, void* winHandle)
{
    mRendererDevice = rendererDevice;
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

    // ensure the subscene reload
    mCurrentSubScene = SIZE_MAX;

    // Basic stuff initialized, try to find the highest subscene possible
    for (mHighestAvailableSubScene = mSubScenes.size() - 1; ; mHighestAvailableSubScene--)
    {
        if (SwitchSubscene(mHighestAvailableSubScene))
            break; // the scene initialized successfully

        if (mHighestAvailableSubScene == 0)
        {
            ReleaseSubsceneResources();
            mHighestAvailableSubScene = SIZE_MAX;
            return false; // we hit the end of our scenes vector, no scene successfully initialized
        }
    }

    mCurrentSubScene = mHighestAvailableSubScene;
    return true;
}

bool Scene::SwitchSubscene(size_t subScene)
{
    if (subScene == mCurrentSubScene)
        return true;

    mCurrentSubScene = SIZE_MAX;
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
    texDesc.mode = BufferMode::GPUOnly;
    texDesc.format = Format::R8G8B8A8_U_Norm;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_RENDERTARGET;
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
    bbDesc.vSync = false;
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