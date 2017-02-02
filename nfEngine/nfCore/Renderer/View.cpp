/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of View class.
 */

#include "PCH.hpp"
#include "View.hpp"
#include "HighLevelRenderer.hpp"
#include "PostProcessRenderer.hpp"
#include "GuiRenderer.hpp"

#include "Engine.hpp"
#include "Utils/ConfigVariable.hpp"
#include "Resources/ResourcesManager.hpp"
#include "Resources/Texture.hpp"

#include "nfCommon/System/Assertion.hpp"
#include "nfCommon/System/Window.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/System/KeyCodes.hpp"

#include "imgui.h"

#include "../../nfCommon/Memory/DefaultAllocator.hpp"

namespace NFE {

namespace {
ConfigVariable<bool> gUseImGui("core/useImGui", true);
} // namespace

namespace Renderer {

View::View(bool useImGui)
{
    mTexture = nullptr;
    mWindow = nullptr;
    mCameraEntity = nullptr;

    if (useImGui && gUseImGui.Get())
    {
        mImGuiWrapper.reset(new ImGuiWrapper);
        RegisterInputListener(mImGuiWrapper.get());
    }
}

View::~View()
{
    if (mImGuiWrapper)
    {
        UnregisterInputListener(mImGuiWrapper.get());
        mImGuiWrapper.reset();
    }

    Release();
}

void View::Release()
{
    if (mTexture != nullptr)
    {
        mTexture->DelRef(this);
        mTexture = nullptr;
    }

    if (mWindow != nullptr)
    {
        mWindow->SetResizeCallback(nullptr, nullptr);
        mWindow = nullptr;
    }
}

void View::RegisterInputListener(Utils::SimpleInputListener* listener)
{
    mInputListeners.push_back(listener);
}

void View::UnregisterInputListener(Utils::SimpleInputListener* listener)
{
    auto iterator = std::find(mInputListeners.begin(), mInputListeners.end(), listener);
    NFE_ASSERT(iterator != mInputListeners.end(), "Input listener not registered");

    mInputListeners.erase(iterator);
}

bool View::OnKeyPressed(const Utils::KeyPressedEvent& event)
{
    for (Utils::SimpleInputListener* listener : mInputListeners)
        if (listener->OnKeyPressed(event))
            return true;
    return false;
}

bool View::OnMouseDown(const Utils::MouseButtonEvent& event)
{
    for (Utils::SimpleInputListener* listener : mInputListeners)
        if (listener->OnMouseDown(event))
            return true;
    return false;
}

bool View::OnMouseUp(const Utils::MouseButtonEvent& event)
{
    for (Utils::SimpleInputListener* listener : mInputListeners)
        if (listener->OnMouseUp(event))
            return true;
    return false;
}

bool View::OnMouseMove(const Utils::MouseMoveEvent& event)
{
    for (Utils::SimpleInputListener* listener : mInputListeners)
        if (listener->OnMouseMove(event))
            return true;
    return false;
}

bool View::OnMouseScroll(int delta)
{
    for (Utils::SimpleInputListener* listener : mInputListeners)
        if (listener->OnMouseScroll(delta))
            return true;
    return false;
}

bool View::OnCharTyped(const char* charUTF8)
{
    for (Utils::SimpleInputListener* listener : mInputListeners)
        if (listener->OnCharTyped(charUTF8))
            return true;
    return false;
}

void View::OnPostRender(GuiRendererContext* context)
{
    // no GUI by default
}

void View::OnDrawImGui(void* state)
{
    UNUSED(state);
}

bool View::SetCamera(Scene::Entity* cameraEntity)
{
    mCameraEntity = cameraEntity;
    return true;
}

// link the view to a window
bool View::SetWindow(Common::Window* window)
{
    Release();

    uint32 width, height;
    window->GetSize(width, height);

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();


    // create backbuffer connected with the window
    BackbufferDesc bbDesc;
    bbDesc.width = width;
    bbDesc.height = height;
    bbDesc.format = renderer->GetBackbufferFormat();
    bbDesc.windowHandle = static_cast<void*>(window->GetHandle());
    bbDesc.vSync = false;
    bbDesc.format = ElementFormat::R8G8B8A8_U_Norm_sRGB;
    mWindowBackbuffer = renderer->GetDevice()->CreateBackbuffer(bbDesc);
    if (mWindowBackbuffer == nullptr)
    {
        LOG_ERROR("Failed to create backbuffer");
        return false;
    }

    if (!InitRenderTarget(mWindowBackbuffer, width, height))
    {
        mWindowBackbuffer.Reset();
        return false;
    }

    window->SetResizeCallback(OnWindowResize, this);
    mWindow = window;

    return true;
}

void View::OnWindowResize(void* userData)
{
    View* view = static_cast<View*>(userData);

    if (view->mWindowBackbuffer != nullptr && view->mWindow != nullptr)
    {
        view->mRenderTarget.Reset();

        uint32 width, height;
        view->mWindow->GetSize(width, height);
        view->mWindowBackbuffer->Resize(width, height);

        if (!view->InitRenderTarget(view->mWindowBackbuffer, width, height))
        {
            view->mWindowBackbuffer.Reset();
        }
    }
}

bool View::InitTemporaryRenderTarget(uint32 width, uint32 height)
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    TextureDesc texDesc;
    texDesc.type = TextureType::Texture2D;
    texDesc.mode = BufferMode::GPUOnly;
    texDesc.width = width;
    texDesc.height = height;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER | NFE_RENDERER_TEXTURE_BIND_RENDERTARGET;
    texDesc.mipmaps = 1;
    texDesc.debugName = "View::mTemporaryBuffer";
    texDesc.format = ElementFormat::R16G16B16A16_Float; // TODO: support for other formats

    mTemporaryBuffer = renderer->GetDevice()->CreateTexture(texDesc);
    if (!mTemporaryBuffer)
    {
        LOG_ERROR("Failed to create temporary buffer texture");
        return false;
    }

    mTemporaryBufferPostprocessBinding = PostProcessRenderer::Get()->CreateTextureBinding(mTemporaryBuffer);
    if (!mTemporaryBufferPostprocessBinding)
    {
        LOG_ERROR("Failed to create binding for temporary buffer texture");
        return false;
    }

    RenderTargetElement rtTarget;
    rtTarget.texture = mTemporaryBuffer;
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    rtDesc.debugName = "View::mTemporaryRenderTarget";

    mTemporaryRenderTarget = renderer->GetDevice()->CreateRenderTarget(rtDesc);
    if (!mTemporaryRenderTarget)
    {
        LOG_ERROR("Failed to create temporary buffer's render target");
        return false;
    }

    return true;
}

bool View::InitRenderTarget(const TexturePtr& texture, uint32 width, uint32 height)
{
    RenderTargetElement rtTarget;
    rtTarget.texture = texture;

    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    rtDesc.debugName = "View::mRenderTarget";

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    mRenderTarget = renderer->GetDevice()->CreateRenderTarget(rtDesc);
    if (mRenderTarget == nullptr)
    {
        LOG_ERROR("Failed to create render target");
        return false;
    }


    mGBuffer.reset(new GeometryBuffer);
    if (!mGBuffer->Resize(width, height))
    {
        mRenderTarget.Reset();
        LOG_ERROR("Failed to create render target");
        return false;
    }

    // TODO: temporary buffer is not needed when post-process is disabled
    if (!InitTemporaryRenderTarget(width, height))
        return false;

    return true;
}

void View::Postprocess(RenderContext* ctx)
{
    // perform post process (if enabled)
    if (mRenderTarget && postProcessParams.enabled && mTemporaryBuffer)
    {
        ToneMappingParameters params;
        params.saturation = postProcessParams.saturation;
        params.noiseFactor = postProcessParams.noiseFactor;
        params.exposureOffset = postProcessParams.exposureOffset;

        PostProcessRendererContext* postProcessContext = ctx->postProcessContext.get();

        PostProcessRenderer::Get()->OnEnter(postProcessContext);
        PostProcessRenderer::Get()->ApplyTonemapping(postProcessContext, params,
                                                     mTemporaryBufferPostprocessBinding,
                                                     mRenderTarget);
        PostProcessRenderer::Get()->OnLeave(postProcessContext);
    }
}

void View::Present()
{
    if (mWindowBackbuffer != nullptr && mRenderTarget != nullptr)
    {
        mWindowBackbuffer->Present();
    }
}

using namespace Resource;

/**
 * This custom load callback function will omit texture file loading.
 */
bool OffscreenViewTextureLoadCallback(ResourceBase* resource, void* userPtr)
{
    UNUSED(resource);
    UNUSED(userPtr);
    return true;
}

// create custom, off-screen render target
Texture* View::SetOffScreen(uint32 width, uint32 height, const char* textureName)
{
    Release();

    ResManager* rm = Engine::GetInstance()->GetResManager();
    mTexture = static_cast<Texture*>(rm->GetResource(textureName, ResourceType::Texture));
    if (mTexture == nullptr) return nullptr;

    mTexture->SetCallbacks(OffscreenViewTextureLoadCallback, nullptr);
    mTexture->Load();
    mTexture->AddRef(this);

    if (!mTexture->CreateAsRenderTarget(width, height, Renderer::ElementFormat::R8G8B8A8_U_Norm))
    {
        Release();
        return nullptr;
    }

    if (!InitRenderTarget(mTexture->GetRendererTexture(), width, height))
    {
        Release();
        return nullptr;
    }

    return mTexture;
}

void View::GetSize(uint32& width, uint32& height)
{
    if (mWindow != nullptr)
    {
        mWindow->GetSize(width, height);
    }

    // TODO: get off-screen view dimensions when it's implemented
}

void View::DrawGui(RenderContext* context)
{
    if (mImGuiWrapper)
    {
        mImGuiWrapper->BeginDrawing(mWindow);
        OnDrawImGui(mImGuiWrapper->GetImGuiInternalState());
        mImGuiWrapper->FinishDrawing(context);
    }
}

void View::DrawViewPropertiesGui()
{
    if (ImGui::Begin("View properties", nullptr, ImVec2(300, 200), 0.8f))
    {
        uint32 width, height;
        GetSize(width, height);
        ImGui::Text("Dimensions: %ix%i", width, height);
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Postprocess parameters", nullptr, true, true))
        {
            ImGui::SliderFloat("Noise Factor", &postProcessParams.noiseFactor, 0.0f, 1.0f);
            ImGui::SliderFloat("Saturation", &postProcessParams.saturation, 0.0f, 3.0f);
            ImGui::SliderFloat("Exposure", &postProcessParams.exposureOffset, -3.0f, 3.0f);
        }
    }
    ImGui::End();
}

} // namespace Renderer
} // namespace NFE
