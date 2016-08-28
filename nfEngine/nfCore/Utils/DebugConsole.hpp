/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Engine's debug console
 */

#pragma once

#include "../Core.hpp"
#include "SimpleInput.hpp"

namespace NFE {
namespace Utils {

class DebugConsole : public SimpleInputListener
{
    NFE_MAKE_NONCOPYABLE(DebugConsole)
    NFE_MAKE_NONMOVEABLE(DebugConsole)

    struct Log
    {
        std::string line;
        uint32 color;
    };

    std::vector<Log> mLogMessages;
    std::vector<std::string> mCommandsHistory;


    bool mVisible;

    uint32 mCursorStart;
    uint32 mCursorEnd;
    std::string mTypedCommand;

    // config:
    uint32 mMargin;
    uint32 mEditBoxHeight;

    std::unique_ptr<Renderer::Font> mFont;

public:
    DebugConsole();

    bool OnKeyPressed(const KeyPressedEvent& event) override;
    bool OnCharTyped(const char* charUTF8) override;

    void Render(Common::Window* window, Renderer::RenderContext* context);
};

} // namespace Utils
} // namespace NFE
