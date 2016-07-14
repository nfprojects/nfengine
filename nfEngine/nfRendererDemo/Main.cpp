/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Main source file of renderer test
 */

#include "PCH.hpp"

#include "Common.hpp"
#include "Scenes/Basic.hpp"
#include "Scenes/DepthStencil.hpp"
#include "Scenes/VertexBuffers.hpp"
#include "Scenes/RenderTargets.hpp"
#include "Scenes/Tessellation.hpp"

#include "../Renderers/RendererInterface/Device.hpp"
#include "nfCommon/Library.hpp"
#include "nfCommon/FileSystem.hpp"
#include "nfCommon/Window.hpp"
#include "nfCommon/Timer.hpp"
#include "nfCommon/KeyCodes.hpp"
#include "nfCommon/Logger.hpp"

#include <algorithm>

using namespace NFE::Renderer;

class DemoWindow : public NFE::Common::Window
{
    size_t mCurrentScene;
    NFE::Common::Library mRendererLib;
    IDevice* mRendererDevice;
    SceneArrayType mScenes;
    float mDeltaTime;

    void SetWindowTitle()
    {
        std::string title = "nfRendererDemo - scene: " +
            std::to_string(mCurrentScene) + " (" + mScenes[mCurrentScene]->GetSceneName() + ')' +
            ", subscene: " + std::to_string(mScenes[mCurrentScene]->GetCurrentSubSceneNumber()) +
            " (" + mScenes[mCurrentScene]->GetCurrentSubSceneName() + "), dt: " +
            std::to_string(mDeltaTime * 1000.0f) + "ms";

        SetTitle(title.c_str());
    }

    void SwitchScene(size_t scene)
    {
        if (scene == mCurrentScene)
            return;

        mScenes[mCurrentScene]->Release();

        if (mScenes[scene]->Init(mRendererDevice, GetHandle()))
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
            mScenes[mCurrentScene]->Init(mRendererDevice, GetHandle());
        }

        SetWindowTitle();
    }

    void SwitchSubScene(size_t subScene)
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
        // TODO: move scene registration to their source files
        // TODO: switching to arbitrary scene (e.g. ommiting a single scene should be
        //       possible, when a feature is not implemented in renderer
        mScenes.push_back(std::unique_ptr<Scene>(new BasicScene));
        mScenes.push_back(std::unique_ptr<Scene>(new DepthStencilScene));
        mScenes.push_back(std::unique_ptr<Scene>(new RenderTargetsScene));
        mScenes.push_back(std::unique_ptr<Scene>(new VertexBuffersScene));
        mScenes.push_back(std::unique_ptr<Scene>(new TessellationScene));
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
     * @param renderer Name of renderer to use
     * @return True on success, false otherwise
     *
     * The method opens Renderer library using NFE::Common::Library class, then extracts the init
     * function for renderer and creates NFE::Renderer::IDevice this way. Should be called only
     * once during Demo lifespan (switching renderers on-the-fly is not supported).
     */
    bool InitRenderer(const std::string& renderer)
    {
        if (!mRendererLib.Open(renderer))
            return false;

        RendererInitFunc proc;
        if (!mRendererLib.GetSymbol(RENDERER_INIT_FUNC, proc))
            return false;

        mRendererDevice = proc();
        if (mRendererDevice == nullptr)
            return false;

        DeviceInfo deviceInfo;
        if (!mRendererDevice->GetDeviceInfo(deviceInfo))
            LOG_ERROR("Failed to get rendering device information");
        else
        {
            LOG_INFO("GPU name: %s", deviceInfo.description.c_str());
            LOG_INFO("GPU info: %s", deviceInfo.misc.c_str());

            std::string features;
            for (size_t i = 0; i < deviceInfo.features.size(); ++i)
            {
                if (i > 0)
                    features += ", ";
                features += deviceInfo.features[i];
            }
            LOG_INFO("GPU features: %s", features.c_str());
        }

        return true;
    }

    /**
     * Initializes Demo with selected scene
     *
     * @param scene Scene index to intialize.
     * @return True on success, false otherwise
     *
     * InitScene initializes selected scene. Should be used only during Demo initialization. To
     * switch scenes, use provided controls in realtime.
     *
     * @see DemoWindow::OnKeyPress
     */
    bool InitScene(size_t scene)
    {
        if (scene >= mScenes.size())
            return false;

        if (!mScenes[scene]->Init(mRendererDevice, GetHandle()))
            return false;

        mCurrentScene = scene;

        SetWindowTitle();
        return true;
    }

    /**
     * Basic loop for drawing current scene
     */
    void DrawLoop()
    {
        NFE::Common::Timer timer;
        timer.Start();

        float timeElapsed = 0.0f;
        int frames = 0;

        while (!IsClosed())
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
        // free scenes
        for (auto& scene : mScenes)
            scene.reset();

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

    void OnKeyPress(NFE::Common::KeyCode key)
    {
        // keep temporarily the IDs
        size_t newSceneId = mCurrentScene;
        size_t newSubSceneId = mScenes[mCurrentScene]->GetCurrentSubSceneNumber();

        switch (key)
        {
        case NFE::Common::KeyCode::Right:
            if (newSceneId >= mScenes.size() - 1)
                return;
            newSceneId++;
            SwitchScene(newSceneId);
            break;

        case NFE::Common::KeyCode::Left:
            if (newSceneId == 0)
                return;
            newSceneId--;
            SwitchScene(newSceneId);
            break;

        case NFE::Common::KeyCode::Up:
            if (newSubSceneId >= mScenes[mCurrentScene]->GetAvailableSubSceneCount())
                return;
            newSubSceneId++;
            SwitchSubScene(newSubSceneId);
            break;

        case NFE::Common::KeyCode::Down:
            if (newSubSceneId == 0)
                return;
            newSubSceneId--;
            SwitchSubScene(newSubSceneId);
            break;

        default:
            break;
        }
    }
};


int main(int argc, char* argv[])
{
    std::string execPath = NFE::Common::FileSystem::GetExecutablePath();
    std::string execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");

    /// select renderer to use - the default will be D3D11
    std::string rend;

    if (argc < 2)
    {
        std::vector<std::string> defBackend = GetDefaultBackend();
        rend = defBackend[0];
        gShaderPathPrefix = defBackend[1];
        gShaderPathExt = defBackend[2];
    }
    else if (D3D11_BACKEND.compare(argv[1]) == 0)
    {
        // we use D3D11 renderer
        rend = D3D11_BACKEND;
        gShaderPathPrefix = HLSL5_SHADER_PATH_PREFIX;
        gShaderPathExt = HLSL5_SHADER_EXTENSION;
    }
    else if (D3D12_BACKEND.compare(argv[1]) == 0)
    {
        rend = D3D12_BACKEND;
        gShaderPathPrefix = HLSL5_SHADER_PATH_PREFIX;
        gShaderPathExt = HLSL5_SHADER_EXTENSION;
    }
    else if (OGL4_BACKEND.compare(argv[1]) == 0)
    {
        rend = OGL4_BACKEND;
        gShaderPathPrefix = GLSL_SHADER_PATH_PREFIX;
        gShaderPathExt = GLSL_SHADER_EXTENSION;
    }
    else if (VK_BACKEND.compare(argv[1]) == 0)
    {
        rend = VK_BACKEND;
        gShaderPathPrefix = GLSL_SHADER_PATH_PREFIX;
        gShaderPathExt = GLSL_SHADER_EXTENSION;
    }
    else
    {
        std::cerr << "Incorrect backend provided" << std::endl;
        return 1;
    }

    DemoWindow window;
    window.SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    window.Open();

    if (!window.InitRenderer(rend))
    {
        std::cerr << "Renderer failed to initialize" << std::endl;
        return 2;
    }

    /// Initial scene to begin with
    if (!window.InitScene(0))
    {
        std::cerr << "Scene failed to initialize" << std::endl;
        return 3;
    }

    window.DrawLoop();
    window.Release();

    // enable memory leak detection at the process exit (Windows only)
#ifdef _CRTDBG_MAP_ALLOC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _CRTDBG_MAP_ALLOC

    return 0;
}
