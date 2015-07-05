/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Main source file of renderer test
 */

#include "PCH.hpp"

#include "Common.hpp"
#include "Scenes.hpp"
#include "RendererUtils.hpp"

#include "../Renderers/RendererInterface/Device.hpp"
#include "../nfCommon/FileSystem.hpp"
#include "../nfCommon/Window.hpp"

using namespace NFE::Common;
using namespace NFE::Renderer;

const std::string NFRENDERER_D3D11_DLL("nfRendererD3D11");

class DemoWindow : public Window
{
    // Current scene ID
    size_t mCurrentScene = 0;
    bool mBlendStateEnabled = false;

    void SwitchScene(size_t id)
    {
        ReleaseResources();
        std::string title = "nfRendererDemo - scene ";
        if (!InitScene(id))
        {
            // add the message
            title += std::to_string(mCurrentScene) + " - Unable to load " + std::to_string(id);

            // assume current scene was already working correctly and reload it
            // otherwise we wouldn't be here in the first place
            InitScene(mCurrentScene);
        }
        else
        {
            mCurrentScene = id;
            title += std::to_string(mCurrentScene);
        }

        SetTitle(title.c_str());
    }

    void SwitchBlendState()
    {
        mBlendStateEnabled = !mBlendStateEnabled;

        ReleaseBlendState();
        std::string title = "nfRendererDemo - scene " + std::to_string(mCurrentScene);
        if (!InitBlendState(mBlendStateEnabled))
            title += " - Unable to switch Blend State";
        else
        {
            title += " - Blending ";
            title += mBlendStateEnabled ? "ON" : "OFF";
        }

        SetTitle(title.c_str());
    }

public:
    bool FindHighestAvailableScene()
    {
        /// check most compilcated available scene
        for (mCurrentScene = GetSceneCount()-1; ; mCurrentScene--)
        {
            if (InitScene(mCurrentScene))
                break; // the scene initialized successfully

            if (mCurrentScene == 0)
                return false; // we hit the end of our scenes vector, no scene successfully inited

            // nope, release it and continue checking
            ReleaseResources();
        }

        std::string title = "nfRendererDemo - scene " + std::to_string(mCurrentScene);
        SetTitle(title.c_str());

        return true;
    }

    void OnKeyPress(int key)
    {
        // keep temporarily the ID
        size_t newSceneId = mCurrentScene;
        switch (key)
        {
        case VK_RIGHT:
            if (newSceneId >= GetSceneCount() - 1)
                return;
            newSceneId++;
            SwitchScene(newSceneId);
            break;

        case VK_LEFT:
            if (newSceneId <= 0)
                return;
            newSceneId--;
            SwitchScene(newSceneId);
            break;

        case 'B':
            SwitchBlendState();
            break;

        default:
            break;
        }
    }
};

DemoWindow gWindow;


int main(int argc, char* argv[])
{
    std::string execPath = NFE::Common::FileSystem::GetExecutablePath();
    NFE::Common::FileSystem::ChangeDirectory(execPath + "/../../../..");

    gWindow.SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    gWindow.SetTitle("nfEngine Renderer Test");
    gWindow.Open();

    /// select renderer to use - the default will be D3D11 renderer
    std::string rend;
    if (argc < 2)
        rend = NFRENDERER_D3D11_DLL;
    else
        rend = argv[1];

    if (!InitRenderer(rend))
        return 1;

    if (!InitRenderTarget(gWindow.GetHandle()))
        return 2;

    if (!gWindow.FindHighestAvailableScene())
    {
        ReleaseResources();
        ReleaseRenderTarget();
        ReleaseRenderer();
        return 3;
    }

    PreDrawLoop();
    while(!gWindow.IsClosed())
    {
        gWindow.ProcessMessages();
        Draw();
    }

    ReleaseResources();
    ReleaseBlendState();
    ReleaseRenderTarget();
    ReleaseRenderer();
    return 0;
}
