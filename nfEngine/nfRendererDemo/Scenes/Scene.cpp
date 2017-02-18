/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Scene base class definition
 */

#include "../PCH.hpp"
#include "Scene.hpp"
#include "nfCommon/nfCommon.hpp"


using namespace NFE;
using namespace Renderer;

Scene::Scene(const std::string& name)
    : mCurrentSubScene(SIZE_MAX)
    , mHighestAvailableSubScene(SIZE_MAX)
    , mName(name)
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
}

bool Scene::Init(IDevice* rendererDevice, void* winHandle)
{
    mRendererDevice = rendererDevice;
    mCommandBuffer.reset(mRendererDevice->CreateCommandRecorder());

    // find suitable back buffer format
    ElementFormat preferredFormats[] =
    {
        ElementFormat::R16G16B16A16_U_Norm,
        ElementFormat::R10G10B10A2_U_Norm,
        ElementFormat::R8G8B8A8_U_Norm,
        ElementFormat::B8G8R8A8_U_Norm,
        ElementFormat::R8G8B8A8_U_Norm_sRGB,
        ElementFormat::B8G8R8A8_U_Norm_sRGB,
    };

    mBackbufferFormat = ElementFormat::Unknown;
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
            return false; // we hit the end of our scenes vector, no scene successfully inited
        }
    }

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

IShader* Scene::CompileShader(const char* path, ShaderType type, ShaderMacro* macros,
                              size_t macrosNum)
{
    ShaderDesc desc;
    desc.type = type;
    desc.path = path;
    desc.macros = macros;
    desc.macrosNum = macrosNum;
    return mRendererDevice->CreateShader(desc);
}