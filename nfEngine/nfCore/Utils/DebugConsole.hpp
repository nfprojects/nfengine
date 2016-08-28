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

class ConsoleLoggerBackend;

class DebugConsole : public SimpleInputListener
{
public:
    using Function = std::function<void()>;

private:
    NFE_MAKE_NONCOPYABLE(DebugConsole)
    NFE_MAKE_NONMOVEABLE(DebugConsole)

    struct LogLine
    {
        std::string line;
        uint32 color;
    };

    ConsoleLoggerBackend* mLoggerBackend;

    std::vector<LogLine> mLoggedLines;
    std::vector<std::string> mCommandsHistory;


    bool mVisible;

    size_t mCursorStart;
    size_t mCursorEnd;
    std::string mTypedCommand;

    // config:
    uint32 mMargin;
    uint32 mEditBoxHeight;
    uint32 mFontSize;

    std::unique_ptr<Renderer::Font> mFont;

    std::map<std::string, Function> mFunctions;

public:
    DebugConsole();
    ~DebugConsole();

    bool OnKeyPressed(const KeyPressedEvent& event) override;
    bool OnCharTyped(const char* charUTF8) override;

    void Render(Common::Window* window, Renderer::RenderContext* context);

    void Log(const std::string& string, uint32 color);

    bool RegisterFunction(const std::string& name, const Function& callback);

    bool Execute(const std::string& command);
};

} // namespace Utils
} // namespace NFE
