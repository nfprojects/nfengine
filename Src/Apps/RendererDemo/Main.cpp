/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author Lookey (costyrra.xl@gmail.com)
 * @brief  Main source file of renderer test
 */

#include "PCH.hpp"

#include "Common.hpp"
#include "Scenes/Scene.hpp"

#include "Engine/Renderers/RendererCommon/Device.hpp"
#include "Engine/Renderers/RendererCommon/Fence.hpp"

#include "Engine/Common/System/Library.hpp"
#include "Engine/Common/FileSystem/FileSystem.hpp"
#include "Engine/Common/System/Window.hpp"
#include "Engine/Common/System/Timer.hpp"
#include "Engine/Common/System/KeyCodes.hpp"
#include "Engine/Common/Logger/Logger.hpp"
#include "Engine/Common/Utils/StringUtils.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"
#include "Engine/Common/Utils/Waitable.hpp"
#include "Engine/Common/Reflection/Types/ReflectionClassType.hpp"

#include <algorithm>
#include <string.h>

using namespace NFE;
using namespace NFE::Common;
using namespace NFE::Renderer;

class DemoWindow : public Window
{
    uint32 mCurrentScene;
    Library mRendererLib;
    IDevice* mRendererDevice;
    CommandQueuePtr mGraphicsQueue;
    CommandQueuePtr mCopyQueue;
    SceneArrayType mScenes;
    float mDeltaTime;

    void SetWindowTitle()
    {
        String title = "nfRendererDemo - scene: " +
            ToString(mCurrentScene) + " (" + mScenes[mCurrentScene]->GetSceneName() + ')' +
            ", subscene: " + ToString(mScenes[mCurrentScene]->GetCurrentSubSceneNumber()) +
            " (" + mScenes[mCurrentScene]->GetCurrentSubSceneName() + "), dt: " +
            ToString(mDeltaTime * 1000.0f) + "ms";

        SetTitle(title.Str());
    }

    void SwitchScene(uint32 scene)
    {
        if (scene == mCurrentScene)
            return;

        // make backbuffer is destroyed before creating new scene (it may be still in be some commandlist)
        mCopyQueue->Signal()->Wait();
        mGraphicsQueue->Signal()->Wait();

        mScenes[mCurrentScene]->Release();

        if (mScenes[scene]->Init(mRendererDevice, mGraphicsQueue, mCopyQueue, GetHandle()))
        {
            /// success - update the scene counter
            mCurrentScene = scene;
        }
        else
        {
            // release failed scene
            mScenes[scene]->Release();

            std::cerr << " - ERROR: Unable to load scene " << std::to_string(scene) << std::endl;

            // assume current scene was already working correctly and reload it
            // otherwise we probably wouldn't be here in the first place
            mScenes[mCurrentScene]->Init(mRendererDevice, mGraphicsQueue, mCopyQueue, GetHandle());
        }

        SetWindowTitle();
    }

    void SwitchSubScene(uint32 subScene)
    {
        if (mScenes[mCurrentScene]->SwitchSubscene(subScene))
        {
            // successful switch, update the title
            SetWindowTitle();
        }
        else
        {
            std::cerr << " - ERROR: Unable to load subcene " + std::to_string(subScene)<< std::endl;

            // assume current subscene was already working correctly and reload it
            // otherwise we probably wouldn't be here in the first place
            subScene = mScenes[mCurrentScene]->GetCurrentSubSceneNumber();
            mScenes[mCurrentScene]->SwitchSubscene(subScene);
        }
    }

public:
    /**
     * Default constructor
     *
     * @param shaderPathPrefix Prefix of Shaders path. Usually a directory which contains shaders.
     * @param shaderExt        Shader files extension
     *
     * The constructor will initialize mScenes vector with existing scenes. Arguments
     * @shaderPathPrefix and @shaderExt are passed to Scene constructors where applicable.
     */
    DemoWindow()
        : mCurrentScene(0)
        , mRendererLib()
        , mRendererDevice(nullptr)
        , mDeltaTime(0.0f)
    {
        RTTI::GetType<Scene>()->ListSubtypes([this](const RTTI::ClassType* type)
        {
            Scene* scenePtr = type->CreateObject<Scene>();
            mScenes.PushBack(UniquePtr<Scene>(scenePtr));
        }, /*skipAbstractTypes*/ true);

        // sort scenes by name
        std::sort(mScenes.Begin(), mScenes.End(), [](const UniquePtr<Scene>& a, const UniquePtr<Scene>& b)
        {
            return a->GetDynamicType()->GetName() < b->GetDynamicType()->GetName();
        });
    }

    /**
     * Destructor, calls Release method.
     *
     * @see DemoWindow::Release
     */
    ~DemoWindow()
    {
        Release();
    }

    /**
     * Initializes NFE Renderer backend
     *
     * @param renderer          Name of renderer to use
     * @param preferredCardId   ID of preferred video card
     * @return True on success, false otherwise
     *
     * The method opens Renderer library using Library class, then extracts the init
     * function for renderer and creates NFE::Renderer::IDevice this way. Should be called only
     * once during Demo lifespan (switching renderers on-the-fly is not supported).
     */
    bool InitRenderer(const Common::String& renderer, int preferredCardId, int debugLevel)
    {
        if (!mRendererLib.Open(renderer))
            return false;

        RendererInitFunc proc;
        if (!mRendererLib.GetSymbol(RENDERER_INIT_FUNC, proc))
            return false;

        DeviceInitParams params;
        params.preferredCardId = preferredCardId;
        params.debugLevel = debugLevel;
        mRendererDevice = proc(&params);
        if (!mRendererDevice)
        {
            return false;
        }

        mGraphicsQueue = mRendererDevice->CreateCommandQueue(CommandQueueType::Graphics, "Graphics");
        if (!mGraphicsQueue)
        {
            return false;
        }

        mCopyQueue = mRendererDevice->CreateCommandQueue(CommandQueueType::Copy, "Copy");
        if (!mCopyQueue)
        {
            return false;
        }

        return true;
    }

    /**
     * Initializes Demo with selected scene
     *
     * @param scene Scene index to initialize.
     * @param subscene Subscene index to initialize. Provide -1 (default) for last properly loaded Subscene.
     * @return True on success, false otherwise
     *
     * InitScene initializes selected scene. Should be used only during Demo initialization. To
     * switch scenes, use provided controls in realtime.
     *
     * @see DemoWindow::OnKeyPress
     */
    bool InitScene(uint32 scene, int32 subscene = -1)
    {
        if (scene >= mScenes.Size())
        {
            NFE_LOG_ERROR("Cannot initialize demo scene - requested scene %d is too high", scene);
            return false;
        }

        if ((subscene >= 0) && (subscene >= static_cast<NFE::int32>(mScenes[scene]->GetSubsceneCount())))
        {
            NFE_LOG_ERROR("Requested subscene %d not available (highest subscene ID is %u)", subscene, mScenes[scene]->GetAvailableSubSceneCount());
            return false;
        }

        if (!mScenes[scene]->Init(mRendererDevice, mGraphicsQueue, mCopyQueue, GetHandle(), subscene))
        {
            NFE_LOG_ERROR("Failed to initialize scene %d", scene);
            return false;
        }

        mCurrentScene = scene;

        SetWindowTitle();
        return true;
    }

    /**
     * Basic loop for drawing current scene
     */
    void DrawLoop()
    {
        Timer timer;
        timer.Start();

        float timeElapsed = 0.0f;
        int frames = 0;

        while (!IsClosed())
        //while (frames < 1)
        {
            float dt = static_cast<float>(timer.Stop());
            timer.Start();

            frames++;
            timeElapsed += dt;

            if (timeElapsed > 1.0f)
            {
                mDeltaTime = timeElapsed / frames;
                timeElapsed = 0;
                frames = 0;
                SetWindowTitle();
            }

            ProcessMessages();
            mScenes[mCurrentScene]->Draw(dt);
        }
    }

    /**
     * Releases scenes, Renderer device and Renderer library.
     */
    void Release()
    {
        mScenes.Clear();

        mCopyQueue.Reset();
        mGraphicsQueue.Reset();

        // free scenes
        for (auto& scene : mScenes)
        {
            scene.Reset();
        }

        // free Renderer
        if (mRendererDevice != nullptr)
        {
            mRendererDevice = nullptr;
            RendererReleaseFunc proc;
            if (!mRendererLib.GetSymbol(RENDERER_RELEASE_FUNC, proc))
                return;
            proc();
        }

        mRendererLib.Close();
    }

    void OnKeyPress(KeyCode key)
    {
        // keep temporarily the IDs
        uint32 newSceneId = mCurrentScene;
        uint32 newSubSceneId = mScenes[mCurrentScene]->GetCurrentSubSceneNumber();

        uint32 numScenes = mScenes[mCurrentScene]->GetAvailableSubSceneCount();

        switch (key)
        {
        case KeyCode::Right:
            if (newSceneId + 1 >= mScenes.Size())
                newSceneId = 0;
            else
                newSceneId++;
            SwitchScene(newSceneId);
            break;

        case KeyCode::Left:
            if (newSceneId == 0)
                newSceneId = mScenes.Size() - 1;
            else
                newSceneId--;
            SwitchScene(newSceneId);
            break;

        case KeyCode::Up:
            newSubSceneId++;
            if (newSubSceneId > numScenes)
                newSubSceneId = 0;
            SwitchSubScene(newSubSceneId);
            break;

        case KeyCode::Down:
            if (newSubSceneId == 0)
                newSubSceneId = numScenes;
            else
                newSubSceneId--;
            SwitchSubScene(newSubSceneId);
            break;

        default:
            break;
        }
    }
};


int InnerMain(int argc, char* argv[])
{
    const String execPath = FileSystem::GetExecutablePath();
    const StringView execDir = FileSystem::GetParentDir(execPath);
    FileSystem::ChangeDirectory(execDir + "/../../..");

    Common::String selectedBackend;
    int initialScene = 0;
    int initialSubscene = -1;
    int selectedCard = -1;
    int debugLevel = 0;

    // TODO use some helper class instead of manual checks
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--renderer") == 0 || strcmp(argv[i], "-r") == 0)
        {
            if (i + 1 < argc)
            {
                selectedBackend = argv[++i];
            }
            else
            {
                NFE_LOG_ERROR("Missing renderer name parameter");
                return 1;
            }
        }
        else if (strcmp(argv[i], "--card") == 0 || strcmp(argv[i], "-c") == 0)
        {
            if (i + 1 < argc)
            {
                if (!Common::FromString(argv[++i], selectedCard))
                {
                    NFE_LOG_ERROR("Invalid card ID parameter");
                    return 1;
                }
            }
            else
            {
                NFE_LOG_ERROR("Missing card ID parameter");
                return 1;
            }
        }
        else if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0)
        {
            if (i + 1 < argc)
            {
                if (!Common::FromString(argv[++i], debugLevel))
                {
                    NFE_LOG_ERROR("Invalid debug level parameter");
                    return 1;
                }
            }
            else
            {
                NFE_LOG_ERROR("Missing debug level parameter");
                return 1;
            }
        }
        else if (strcmp(argv[i], "--scene") == 0 || strcmp(argv[i], "-s") == 0)
        {
            if (i + 1 < argc)
            {
                NFE::Common::StringView sceneId(argv[++i]);
                NFE::uint32 split = sceneId.FindFirst(':');
                if (split == Common::StringView::END())
                {
                    if (!Common::FromString(sceneId, initialScene))
                    {
                        NFE_LOG_INFO("Invalid scene number parameter");
                        return 1;
                    }

                    NFE_LOG_INFO("Requested scene %d", initialScene);
                }
                else
                {
                    NFE::Common::StringView sceneStr = sceneId.Range(0, split);
                    NFE::Common::StringView subsceneStr = sceneId.Range(split + 1, sceneId.Length() - split - 1);

                    if (!Common::FromString(sceneStr, initialScene))
                    {
                        NFE_LOG_ERROR("Invalid scene ID parameter");
                        return 1;
                    }

                    if (!Common::FromString(subsceneStr, initialSubscene))
                    {
                        NFE_LOG_ERROR("Invalid subscene ID parameter");
                        return 1;
                    }

                    NFE_LOG_INFO("Requested scene:subscene = %d:%d", initialScene, initialSubscene);
                }
            }
            else
            {
                NFE_LOG_ERROR("Missing command line parameter");
                return 1;
            }
        }
        else
        {
            NFE_LOG_ERROR("Unknown command line parameter: %s", argv[i]);
            return 1;
        }
    }

    if (selectedBackend.Empty())
    {
        const auto& defBackend = GetDefaultBackend();
        selectedBackend = defBackend[0];
        gShaderPathPrefix = defBackend[1];
        gShaderPathExt = defBackend[2];
    }
    else if (D3D12_BACKEND == selectedBackend)
    {
        gShaderPathPrefix = HLSL5_SHADER_PATH_PREFIX;
        gShaderPathExt = HLSL5_SHADER_EXTENSION;
    }
    else if (VK_BACKEND == selectedBackend)
    {
        gShaderPathPrefix = GLSL_SHADER_PATH_PREFIX;
        gShaderPathExt = GLSL_SHADER_EXTENSION;
    }
    else
    {
        NFE_LOG_ERROR("Incorrect backend provided");
        return 1;
    }

    DemoWindow window;
    if (!window.Init())
    {
        NFE_LOG_ERROR("Failed to initialize Window");
        return 4;
    }

    window.SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    if (!window.Open())
    {
        NFE_LOG_ERROR("Failed to open Window");
        return 5;
    }

    if (!window.InitRenderer(selectedBackend, selectedCard, debugLevel))
    {
        NFE_LOG_ERROR("Renderer failed to initialize");
        return 2;
    }

    /// Initial scene to begin with
    if (!window.InitScene(initialScene, initialSubscene))
    {
        NFE_LOG_ERROR("Scene failed to initialize");
        return 3;
    }

    window.DrawLoop();

    return 0;
}

int main(int argc, char* argv[])
{
    if (!InitSubsystems())
    {
        ShutdownSubsystems();
        return -1;
    }

    int ret = InnerMain(argc, argv);

    ShutdownSubsystems();

    // enable memory leak detection at the process exit (Windows only)
#ifdef _CRTDBG_MAP_ALLOC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _CRTDBG_MAP_ALLOC

    return ret;
}
