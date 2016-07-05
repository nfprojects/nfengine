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
#include "Resources/ResourcesManager.hpp"
#include "Resources/Texture.hpp"

#include "nfCommon/Window.hpp"
#include "nfCommon/Logger.hpp"
#include "nfCommon/KeyCodes.hpp"

#include "imgui.h"


namespace NFE {
namespace Renderer {

View::View()
{
    mScene = nullptr;
    mTexture = nullptr;
    mWindow = nullptr;
    mCameraEntity = 0;

    InitImGui();
}

View::~View()
{
    if (mImGuiState)
    {
        ImGui::SetInternalState(mImGuiState, true);
        ImGui::Shutdown();
        mImGuiState = nullptr;
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

void View::OnPostRender(RenderContext* context)
{
    // no GUI by default
}

void View::OnDrawImGui(void* state)
{
    UNUSED(state);
}

bool View::SetCamera(Scene::SceneManager* scene, Scene::EntityID cameraEntity)
{
    mScene = scene;
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
    bbDesc.windowHandle = static_cast<void*>(window->GetHandle());
    bbDesc.vSync = false;
    mWindowBackbuffer.reset(renderer->GetDevice()->CreateBackbuffer(bbDesc));
    if (mWindowBackbuffer == nullptr)
    {
        LOG_ERROR("Failed to create backbuffer");
        return false;
    }

    if (!InitRenderTarget(mWindowBackbuffer.get(), width, height))
    {
        mWindowBackbuffer.reset();
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
        view->mRenderTarget.reset();

        uint32 width, height;
        view->mWindow->GetSize(width, height);
        view->mWindowBackbuffer->Resize(width, height);

        if (!view->InitRenderTarget(view->mWindowBackbuffer.get(), width, height))
        {
            view->mWindowBackbuffer.reset();
        }
    }
}

bool View::InitTemporaryRenderTarget(uint32 width, uint32 height)
{
    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();

    TextureDesc texDesc;
    texDesc.type = TextureType::Texture2D;
    texDesc.access = BufferAccess::GPU_ReadWrite;
    texDesc.width = width;
    texDesc.height = height;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER | NFE_RENDERER_TEXTURE_BIND_RENDERTARGET;
    texDesc.mipmaps = 1;
    texDesc.debugName = "View::mTemporaryBuffer";
    texDesc.format = ElementFormat::Float_16; // TODO: support for other formats
    texDesc.texelSize = 4;

    mTemporaryBuffer.reset(renderer->GetDevice()->CreateTexture(texDesc));
    if (!mTemporaryBuffer)
    {
        LOG_ERROR("Failed to create temporary buffer texture");
        return false;
    }

    RenderTargetElement rtTarget;
    rtTarget.texture = mTemporaryBuffer.get();
    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    rtDesc.debugName = "View::mTemporaryRenderTarget";

    mTemporaryRenderTarget.reset(renderer->GetDevice()->CreateRenderTarget(rtDesc));
    if (!mTemporaryRenderTarget)
    {
        LOG_ERROR("Failed to create temporary buffer's render target");
        return false;
    }

    return true;
}

bool View::InitRenderTarget(ITexture* texture, uint32 width, uint32 height)
{
    RenderTargetElement rtTarget;
    rtTarget.texture = texture;

    RenderTargetDesc rtDesc;
    rtDesc.numTargets = 1;
    rtDesc.targets = &rtTarget;
    rtDesc.debugName = "View::mRenderTarget";

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    mRenderTarget.reset(renderer->GetDevice()->CreateRenderTarget(rtDesc));
    if (mRenderTarget == nullptr)
    {
        LOG_ERROR("Failed to create render target");
        return false;
    }


    mGBuffer.reset(new GeometryBuffer);
    if (!mGBuffer->Resize(width, height))
    {
        mRenderTarget.reset();
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

        PostProcessRenderer::Get()->Enter(ctx);
        PostProcessRenderer::Get()->ApplyTonemapping(ctx, params,
                                                     mTemporaryBuffer.get(),
                                                     mRenderTarget.get());
        PostProcessRenderer::Get()->Leave(ctx);
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

    if (!mTexture->CreateAsRenderTarget(width, height, Renderer::ElementFormat::Uint_8_norm))
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

bool View::InitImGui()
{
    // create and initialize internal state for ImGui
    mImGuiState = malloc(ImGui::GetInternalStateSize());
    ImGui::SetInternalState(mImGuiState, true);

    ImGuiIO& io = ImGui::GetIO();

    /// get texture atlas from ImGui
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    TextureDataDesc texDataDesc;
    texDataDesc.lineSize = texDataDesc.sliceSize = width * 4 * sizeof(uint8);
    texDataDesc.data = pixels;

    TextureDesc texDesc;
    texDesc.type = TextureType::Texture2D;
    texDesc.access = BufferAccess::GPU_ReadOnly;
    texDesc.width = width;
    texDesc.height = height;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    texDesc.mipmaps = 1;
    texDesc.dataDesc = &texDataDesc;
    texDesc.format = ElementFormat::Uint_8_norm;
    texDesc.texelSize = 4;
    texDesc.debugName = "GuiRenderer::mImGuiTexture";

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    mImGuiTexture.reset(renderer->GetDevice()->CreateTexture(texDesc));
    if (!mImGuiTexture)
        return false;

    io.Fonts->TexID = mImGuiTexture.get();
    io.Fonts->ClearInputData();
    io.Fonts->ClearTexData();
    io.IniFilename = nullptr;  // don't use INI file

    /**
     * Configure style.
     * TODO: move to engine's configuration.
     */
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowMinSize             = ImVec2(160, 20);
    style.FramePadding              = ImVec2(4, 4);
    style.ItemSpacing               = ImVec2(6, 2);
    style.ItemInnerSpacing          = ImVec2(6, 4);
    style.Alpha                     = 0.9f;
    style.WindowFillAlphaDefault    = 0.9f;
    style.WindowRounding            = 3.0f;
    style.FrameRounding             = 2.0f;
    style.IndentSpacing             = 6.0f;
    style.ItemInnerSpacing          = ImVec2(2, 4);
    style.ColumnsMinSpacing         = 50.0f;
    style.GrabMinSize               = 8.0f;
    style.GrabRounding              = 2.0f;
    style.ScrollbarSize             = 12.0f;
    style.ScrollbarRounding         = 16.0f;
    style.Colors[ImGuiCol_MenuBarBg]            = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
    style.Colors[ImGuiCol_Text]                 = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab]        = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.75f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(1.0f, 1.0f, 1.0f, 0.25f);
    style.Colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.2f, 0.25f, 0.3f, 0.8f);
    style.Colors[ImGuiCol_Header]               = ImVec4(0.4f, 0.5f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive]         = ImVec4(0.3f, 0.4f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered]        = ImVec4(0.5f, 0.6f, 0.7f, 1.0f);
    style.Colors[ImGuiCol_TitleBg]              = ImVec4(0.4f, 0.5f, 0.6f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive]        = ImVec4(0.5f, 0.55f, 0.66f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.3f, 0.4f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_CloseButton]          = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
    style.Colors[ImGuiCol_CloseButtonActive]    = ImVec4(0.5f, 0.05f, 0.05f, 0.9f);
    style.Colors[ImGuiCol_CloseButtonHovered]   = ImVec4(1.0f, 0.1f, 0.1f, 0.9f);
    style.Colors[ImGuiCol_Button]               = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive]         = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered]        = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

    /**
     * Keyboard mapping.
     */
    io.KeyMap[ImGuiKey_Tab]         = static_cast<int>(Common::KeyCode::Tab);
    io.KeyMap[ImGuiKey_LeftArrow]   = static_cast<int>(Common::KeyCode::Left);
    io.KeyMap[ImGuiKey_RightArrow]  = static_cast<int>(Common::KeyCode::Right);
    io.KeyMap[ImGuiKey_UpArrow]     = static_cast<int>(Common::KeyCode::Up);
    io.KeyMap[ImGuiKey_DownArrow]   = static_cast<int>(Common::KeyCode::Down);
    io.KeyMap[ImGuiKey_PageUp]      = static_cast<int>(Common::KeyCode::PageUp);
    io.KeyMap[ImGuiKey_PageDown]    = static_cast<int>(Common::KeyCode::PageDown);
    io.KeyMap[ImGuiKey_Home]        = static_cast<int>(Common::KeyCode::Home);
    io.KeyMap[ImGuiKey_End]         = static_cast<int>(Common::KeyCode::End);
    io.KeyMap[ImGuiKey_Delete]      = static_cast<int>(Common::KeyCode::Delete);
    io.KeyMap[ImGuiKey_Backspace]   = static_cast<int>(Common::KeyCode::Backspace);
    io.KeyMap[ImGuiKey_Enter]       = static_cast<int>(Common::KeyCode::Enter);
    io.KeyMap[ImGuiKey_Escape]      = static_cast<int>(Common::KeyCode::Escape);
    io.KeyMap[ImGuiKey_A]           = static_cast<int>(Common::KeyCode::A);
    io.KeyMap[ImGuiKey_C]           = static_cast<int>(Common::KeyCode::C);
    io.KeyMap[ImGuiKey_V]           = static_cast<int>(Common::KeyCode::V);
    io.KeyMap[ImGuiKey_X]           = static_cast<int>(Common::KeyCode::X);
    io.KeyMap[ImGuiKey_Y]           = static_cast<int>(Common::KeyCode::Y);
    io.KeyMap[ImGuiKey_Z]           = static_cast<int>(Common::KeyCode::Z);

    return true;
}

void View::UpdateGui()
{
    ImGui::SetInternalState(mImGuiState);

    uint32 width, height;
    GetSize(width, height);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = static_cast<float>(width);
    io.DisplaySize.y = static_cast<float>(height);
    io.RenderDrawListsFn = nullptr;

    // process input
    if (mWindow)
    {
        io.MouseDown[0] = mWindow->IsMouseButtonDown(0);
        io.MouseDown[1] = mWindow->IsMouseButtonDown(1);
        io.MouseDown[2] = mWindow->IsMouseButtonDown(2);

        int x, y;
        mWindow->GetMousePosition(x, y);
        io.MousePos.x = static_cast<float>(x);
        io.MousePos.y = static_cast<float>(y);
        io.MouseWheel = static_cast<float>(mWindow->GetMouseWheelDelta());

        io.KeyCtrl = mWindow->IsKeyPressed(Common::KeyCode::ControlLeft);
        io.KeyShift = mWindow->IsKeyPressed(Common::KeyCode::ShiftLeft);
        io.KeyAlt = mWindow->IsKeyPressed(Common::KeyCode::AltLeft);
        for (unsigned int i = 0; i < NFE_WINDOW_KEYS_NUM; ++i)
            io.KeysDown[i] = mWindow->IsKeyPressed(static_cast<Common::KeyCode>(i));

        io.AddInputCharactersUTF8(mWindow->GetInputCharacters());
    }

    ImGui::NewFrame();
    OnDrawImGui(mImGuiState);
}

void View::DrawGui(RenderContext* context)
{
    ITexture* imGuiTexture = mImGuiTexture.get();
    context->commandBuffer->SetTextures(&imGuiTexture, 1, ShaderType::Pixel);
    GuiRenderer::Get()->DrawImGui(context);
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
