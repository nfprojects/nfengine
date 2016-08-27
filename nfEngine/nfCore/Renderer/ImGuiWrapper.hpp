/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Declarations of ImGuiWrapper.
 */

#pragma once

#include "../Core.hpp"
#include "../Utils/SimpleInput.hpp"
#include "../../Renderers/RendererInterface/Texture.hpp"
#include "../../Renderers/RendererInterface/ResourceBinding.hpp"


namespace NFE {
namespace Renderer {

class ImGuiWrapper final : public Utils::SimpleInputListener
{
    // ImGui internal state
    void* mDefaultImGuiState;
    void* mImGuiState;
    std::unique_ptr<ITexture> mImGuiTexture;
    std::unique_ptr<IResourceBindingInstance> mImGuiTextureBinding; // For GUI Renderer

    bool InitImGui();

public:
    ImGuiWrapper();
    ~ImGuiWrapper();

    NFE_INLINE void* GetImGuiInternalState() const
    {
        return mImGuiState;
    }

    // SimpleInputListener
    bool OnKeyPressed(const Utils::KeyPressedEvent& event) override;
    bool OnMouseDown(const Utils::MouseButtonEvent& event) override;
    bool OnMouseUp(const Utils::MouseButtonEvent& event) override;
    bool OnMouseMove(const Utils::MouseMoveEvent& event) override;
    bool OnMouseScroll(int delta) override;
    bool OnCharTyped(const char* charUTF8) override;

    // must be called before building GUI
    void BeginDrawing(Common::Window* window);

    // must be called after building GUI
    void FinishDrawing(RenderContext* context);
};

} // namespace Renderer
} // namespace NFE
