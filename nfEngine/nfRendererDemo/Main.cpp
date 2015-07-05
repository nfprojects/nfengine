/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Main source file of renderer test
 */

#include "PCH.hpp"

#include "Common.hpp"
#include "BasicScene.hpp"

#include "../Renderers/RendererInterface/Device.hpp"
#include "../nfCommon/Library.hpp"
#include "../nfCommon/FileSystem.hpp"
#include "../nfCommon/Window.hpp"

using namespace NFE::Common;
using namespace NFE::Renderer;

class DemoWindow : public Window
{
    size_t mCurrentScene;
    size_t mCurrentSubScene;
    Library mRendererLib;
    IDevice* mRendererDevice;
    SceneArrayType mScenes;

    void SwitchScene(size_t scene)
    {
        if (scene == mCurrentScene)
            return;

        mScenes[mCurrentScene]->Release();

        std::string title = "nfRendererDemo - scene ";
        if (!mScenes[scene]->Init(mRendererDevice, GetHandle()))
        {
            /// success - update the scene counter and the title
            mCurrentScene = scene;
            title += std::to_string(mCurrentScene) + ':' +
                     std::to_string(mScenes[scene]->GetAvailableSubSceneCount());
        }
        else
        {
            // release failed scene
            mScenes[scene]->Release();

            // construct fail message
            title += std::to_string(mCurrentScene) + ':' + std::to_string(mCurrentSubScene) +
                     " - Unable to load scene " + std::to_string(scene);

            // assume current scene was already working correctly and reload it
            // otherwise we probably wouldn't be here in the first place
            mScenes[mCurrentScene]->Init(mRendererDevice, GetHandle());
        }

        SetTitle(title.c_str());
    }

    void SwitchSubScene(size_t subScene)
    {
        if (subScene == mCurrentSubScene)
            return;

        std::string title = "nfRendererDemo - scene " + std::to_string(mCurrentScene) + ':';
        if (mScenes[mCurrentScene]->SwitchSubScene(subScene))
        {
            // successful switch, update the title
            mCurrentSubScene = subScene;
            title += std::to_string(mCurrentSubScene);
        }
        else
        {
            // construct fail message
            title += std::to_string(mCurrentScene) + ':' + std::to_string(mCurrentSubScene) +
                     " - Unable to load subcene " + std::to_string(subScene);

            // assume current subscene was already working correctly and reload it
            // otherwise we probably wouldn't be here in the first place
            mScenes[mCurrentScene]->SwitchSubScene(mCurrentSubScene);
        }

        SetTitle(title.c_str());
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
    DemoWindow(const std::string& shaderPathPrefix, const std::string shaderExt)
        : mCurrentScene(0)
        , mRendererLib()
        , mRendererDevice(nullptr)
    {
        // the first scene ever made
        mScenes.push_back(std::unique_ptr<Scene>(new BasicScene(shaderPathPrefix, shaderExt)));
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
        mCurrentSubScene = mScenes[scene]->GetAvailableSubSceneCount();
        // assume Scene::Init will preinitialize highest available Subscene
        std::string title = "nfRendererDemo - scene " + std::to_string(mCurrentScene) +
                            ':' + std::to_string(mCurrentSubScene);
        SetTitle(title.c_str());
        return true;
    }

    /**
     * Basic loop for drawing current scene
     */
    void DrawLoop()
    {
        while(!IsClosed())
        {
            ProcessMessages();
            mScenes[mCurrentScene]->Draw();
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

    void OnKeyPress(int key)
    {
        // keep temporarily the IDs
        size_t newSceneId = mCurrentScene;
        size_t newSubSceneId = mCurrentSubScene;
        switch (key)
        {
        case VK_RIGHT:
            if (newSceneId >= mScenes.size() - 1)
                return;
            newSceneId++;
            SwitchScene(newSceneId);
            break;

        case VK_LEFT:
            if (newSceneId == 0)
                return;
            newSceneId--;
            SwitchScene(newSceneId);
            break;

        case VK_UP:
            if (newSubSceneId >= mScenes[mCurrentScene]->GetAvailableSubSceneCount())
                return;
            newSubSceneId++;
            SwitchSubScene(newSubSceneId);
            break;

        case VK_DOWN:
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
    NFE::Common::FileSystem::ChangeDirectory(execPath + "/../../../..");

    /// select renderer to use - the default will be D3D11
    std::string rend;
    std::string shaderPathPrefix;
    std::string shaderExt;

    if ((argc < 2) || (D3D11_BACKEND.compare(argv[1]) == 0))
    {
        // we use D3D11 renderer
        rend = D3D11_BACKEND;
        shaderPathPrefix = D3D11_SHADER_PATH_PREFIX;
        shaderExt = D3D11_SHADER_EXTENSION;
    }
    else if (OGL4_BACKEND.compare(argv[1]))
    {
        rend = OGL4_BACKEND;
        shaderPathPrefix = OGL4_SHADER_PATH_PREFIX;
        shaderExt = OGL4_SHADER_EXTENSION;
    }
    else
    {
        std::cerr << "Incorrect backend provided" << std::endl;
        return 1;
    }

    DemoWindow window(shaderPathPrefix, shaderExt);
    window.SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    window.Open();

    if (!window.InitRenderer(rend))
        return 2;

    /// Initial scene to begin with
    if (!window.InitScene(0))
        return 3;

    window.DrawLoop();

    window.Release();
    return 0;
}
