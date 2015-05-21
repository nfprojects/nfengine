/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of View class.
 */

#include "../PCH.hpp"
#include "View.hpp"
#include "HighLevelRenderer.hpp"

#include "../Globals.hpp"
#include "../ResourcesManager.hpp"
#include "../Texture.hpp"

#include "Window.hpp"
#include "Logger.hpp"
#include "Image.hpp"

namespace NFE {
namespace Renderer {

View::View()
{
    RT = nullptr;
    camera = nullptr;
    mTexture = nullptr;
}

View::~View()
{
    Release();
}

void View::Release()
{
    if (RT != nullptr)
    {
        delete RT;
        RT = 0;
    }

    if (mTexture != nullptr)
    {
        mTexture->DelRef(this);
        //  mTexture->Unload();
        mTexture = nullptr;
    }
}

void View::OnPostRender(RenderContext* context)
{
    // no GUI by default
}

Result View::SetCamera(Scene::Camera* pCamera)
{
    camera = pCamera;
    return Result::OK;
}

Scene::Camera* View::GetCamera() const
{
    return camera;
}

// link the view to a window
Result View::SetWindow(Common::Window* window)
{
    Release();

    RT = nullptr; // TODO
    if (RT == nullptr)
    {
        LOG_ERROR("Memory allocation failed");
        return Result::AllocationError;
    }

    uint32 width, height;
    window->GetSize(width, height);
    // RT->Init(width, height, pWindow); // TODO
    return Result::OK;
}

using namespace Resource;

// create custom, off-screen render target
Texture* View::SetOffScreen(uint32 width, uint32 height, const char* textureName)
{
    Release();

    mTexture = (Texture*)g_pResManager->GetResource(textureName, ResourceType::Texture);
    if (mTexture == nullptr) return nullptr;

    mTexture->Load();
    mTexture->AddRef(this);
    RT = mTexture->CreateRendertarget(width, height, Common::ImageFormat::RGBA_Float);

    return mTexture;
}

} // namespace Renderer
} // namespace NFE
