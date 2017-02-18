/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of ImGuiWrapper.
 */

#include "../PCH.hpp"
#include "ImGuiWrapper.hpp"
#include "GuiRenderer.hpp"

#include "Engine.hpp"
#include "Resources/ResourcesManager.hpp"
#include "Resources/Texture.hpp"

#include "nfCommon/Window.hpp"
#include "nfCommon/Logger.hpp"
#include "nfCommon/KeyCodes.hpp"

#include "imgui.h"


#include "../../nfCommon/Memory/DefaultAllocator.hpp"

namespace NFE {
namespace Renderer {

ImGuiWrapper::ImGuiWrapper()
    : mDefaultImGuiState(nullptr)
    , mImGuiState(nullptr)
{
    InitImGui();
}

ImGuiWrapper::~ImGuiWrapper()
{
    if (mImGuiState)
    {
        ImGui::SetInternalState(mImGuiState);
        ImGui::Shutdown();
        ImGui::SetInternalState(mDefaultImGuiState);

        NFE_FREE(mImGuiState);
        mImGuiState = nullptr;
    }
}

bool ImGuiWrapper::OnKeyPressed(const Utils::KeyPressedEvent& event)
{
    ImGui::SetInternalState(mImGuiState);
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[static_cast<int>(event.key)] = true;

    return io.WantCaptureKeyboard;
}

bool ImGuiWrapper::OnMouseDown(const Utils::MouseButtonEvent& event)
{
    ImGui::SetInternalState(mImGuiState);
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = static_cast<float>(event.x);
    io.MousePos.y = static_cast<float>(event.y);
    io.MouseDown[event.mouseButton] = true;

    return io.WantCaptureMouse;
}

bool ImGuiWrapper::OnMouseUp(const Utils::MouseButtonEvent& event)
{
    ImGui::SetInternalState(mImGuiState);
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[event.mouseButton] = false;

    return io.WantCaptureMouse;
}

bool ImGuiWrapper::OnMouseMove(const Utils::MouseMoveEvent& event)
{
    ImGui::SetInternalState(mImGuiState);
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = static_cast<float>(event.x);
    io.MousePos.y = static_cast<float>(event.y);

    return io.WantCaptureMouse;
}

bool ImGuiWrapper::OnMouseScroll(int delta)
{
    ImGui::SetInternalState(mImGuiState);
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel = static_cast<float>(delta);
    return io.WantCaptureMouse;
}

bool ImGuiWrapper::OnCharTyped(const char* charUTF8)
{
    ImGui::SetInternalState(mImGuiState);
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput)
    {
        io.AddInputCharactersUTF8(charUTF8);
        return true;
    }

    return false;
}

bool ImGuiWrapper::InitImGui()
{
    mDefaultImGuiState = ImGui::GetInternalState();

    // create and initialize internal state for ImGui
    mImGuiState = NFE_MALLOC(ImGui::GetInternalStateSize(), 1);
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
    texDesc.mode = BufferMode::Static;
    texDesc.width = width;
    texDesc.height = height;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    texDesc.mipmaps = 1;
    texDesc.dataDesc = &texDataDesc;
    texDesc.format = ElementFormat::R8G8B8A8_U_Norm;
    texDesc.debugName = "GuiRenderer::mImGuiTexture";

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    mImGuiTexture = renderer->GetDevice()->CreateTexture(texDesc);
    if (!mImGuiTexture)
        return false;

    mImGuiTextureBinding = GuiRenderer::Get()->CreateTextureBinding(mImGuiTexture);
    if (!mImGuiTextureBinding)
        return false;

    io.Fonts->TexID = mImGuiTextureBinding.get();
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

    // convert to linear color space
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
    {
        style.Colors[i].x = powf(style.Colors[i].x, 2.2f);
        style.Colors[i].y = powf(style.Colors[i].y, 2.2f);
        style.Colors[i].z = powf(style.Colors[i].z, 2.2f);
    }

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

void ImGuiWrapper::BeginDrawing(Common::Window* window)
{
    ImGui::SetInternalState(mImGuiState);

    ImGuiIO& io = ImGui::GetIO();

    uint32 width, height;
    window->GetSize(width, height);
    io.DisplaySize.x = static_cast<float>(width);
    io.DisplaySize.y = static_cast<float>(height);
    io.RenderDrawListsFn = nullptr;
    io.KeyCtrl = window->IsKeyPressed(Common::KeyCode::ControlLeft);
    io.KeyShift = window->IsKeyPressed(Common::KeyCode::ShiftLeft);
    io.KeyAlt = window->IsKeyPressed(Common::KeyCode::AltLeft);

    ImGui::NewFrame();
}

void ImGuiWrapper::FinishDrawing(RenderContext* context)
{
    ImGui::SetInternalState(mImGuiState);

    GuiRenderer::Get()->DrawImGui(context->guiContext.get(), mImGuiTextureBinding);

    ImGuiIO& io = ImGui::GetIO();

    // clear keyboard state before next frame
    for (unsigned int i = 0; i < NFE_WINDOW_KEYS_NUM; ++i)
        io.KeysDown[i] = false;
}

} // namespace Renderer
} // namespace NFE
