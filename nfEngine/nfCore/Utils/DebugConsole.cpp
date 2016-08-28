/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Engine's debug console
 */

#include "../PCH.hpp"

#include "DebugConsole.hpp"
#include "ConfigManager.hpp"
#include "../Renderer/GuiRenderer.hpp"

#include "nfCommon/Logger.hpp"
#include "nfCommon/Window.hpp"

namespace NFE {
namespace Utils {

using namespace Renderer;
using namespace Common;


class ConsoleLoggerBackend : public LoggerBackend
{
    DebugConsole* mConsole;
    std::mutex mMutex;

public:
    ConsoleLoggerBackend()
        : mConsole(nullptr)
    {
    }

    void AttachToConsole(DebugConsole* console)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mConsole = console;
    }

    void Log(LogType type, const char* srcFile, int line, const char* str, double timeElapsed) override
    {
        std::lock_guard<std::mutex> lock(mMutex);

        if (!mConsole)
            return;

        uint32 color = 0xFFD0D0D0;
        switch (type)
        {
        case LogType::Debug:
            color = 0xFFFF8000;
            break;
        case LogType::OK:
            color = 0xFF00FF00;
            break;
        case LogType::Warning:
            color = 0xFF00FFFF;
            break;
        case LogType::Error:
        case LogType::Fatal:
            color = 0xFF0000FF;
            break;
        }

        mConsole->Log("[" + std::to_string(timeElapsed) + "] " + str, color);
    }
};

bool gLoggerBackendDebugConsoleRegistered = Logger::RegisterBackend("DebugConsole", std::make_unique<ConsoleLoggerBackend>());


DebugConsole::DebugConsole()
    : mVisible(false)
    , mCursorStart(0)
    , mCursorEnd(0)
{
    mMargin = 6;
    mEditBoxHeight = 20;
    mFontSize = 10;

    mFont.reset(new Font);
    mFont->Init("nfEngineDemo/Data/Fonts/Inconsolata.otf", mFontSize);
    // mFont->Init("nfEngineDemo/Data/Fonts/tahoma.ttf", mFontSize);

    mLoggerBackend = dynamic_cast<ConsoleLoggerBackend*>(Logger::GetBackend("DebugConsole"));
    mLoggerBackend->AttachToConsole(this);
    mLoggerBackend->Enable(true);

    auto crashFunction = []()
    {
        volatile int* pointer = nullptr;
        *pointer = 0x12345678;
    };
    RegisterFunction("crash", crashFunction);
}

bool DebugConsole::RegisterFunction(const std::string& name, const Function& callback)
{
    mFunctions[name] = callback;
    return true;
}

DebugConsole::~DebugConsole()
{
    if (mLoggerBackend)
    {
        mLoggerBackend->AttachToConsole(nullptr);
        mLoggerBackend = nullptr;
    }
}

bool DebugConsole::OnKeyPressed(const KeyPressedEvent& event)
{
    if (event.key == Common::KeyCode::Tilde)
    {
        mVisible ^= true;
        return true;
    }
    else if (event.key == Common::KeyCode::Enter && !mTypedCommand.empty())
    {
        Execute(mTypedCommand);

        mTypedCommand.clear();
        mCursorEnd = mCursorStart = 0;
    }
    else if (event.key == Common::KeyCode::Left) // move cursor left
    {
        mCursorStart = (mCursorStart > 0) ? (mCursorStart - 1) : 0;
        mCursorEnd = mCursorStart;
    }
    else if (event.key == Common::KeyCode::Right) // move cursor right
    {
        mCursorEnd = (mCursorEnd < mTypedCommand.size()) ? (mCursorEnd + 1) : mTypedCommand.size();
        mCursorStart = mCursorEnd;
    }
    else if (event.key == Common::KeyCode::End) // move cursor to the end
    {
        mCursorEnd = mTypedCommand.size();
        mCursorStart = mCursorEnd;
    }
    else if (event.key == Common::KeyCode::Home) // move cursor to the beginning
    {
        mCursorStart = mCursorEnd = 0;
    }
    else if (event.key == Common::KeyCode::A && event.isCtrlPressed) // select all
    {
        mCursorStart = 0;
        mCursorEnd = mTypedCommand.size();
    }
    else if (event.key == Common::KeyCode::Backspace)
    {
        if (mCursorEnd > mCursorStart) // remove selection
        {
            mTypedCommand.erase(mCursorStart, mCursorEnd - mCursorStart);
            mCursorEnd = mCursorStart;
        }
        else if (mCursorStart > 0) // delete char before cursor
        {
            mTypedCommand.erase(mCursorStart - 1, 1);
            mCursorEnd = --mCursorStart;
        }
    }
    else if (event.key == Common::KeyCode::Delete)
    {
        if (mCursorEnd > mCursorStart) // remove selection
        {
            mTypedCommand.erase(mCursorStart, mCursorEnd - mCursorStart);
            mCursorEnd = mCursorStart;
        }
        else if (mCursorEnd < mTypedCommand.size()) // delete char after cursor
        {
            mTypedCommand.erase(mCursorEnd, 1);
        }
    }

    return mVisible;
}

bool DebugConsole::OnCharTyped(const char* charUTF8)
{
    if (!mVisible)
        return false;

    if (strcmp(charUTF8, "`") == 0)
        return true;

    mTypedCommand.erase(mCursorStart, mCursorEnd - mCursorStart);
    mTypedCommand.insert(mCursorStart, charUTF8);
    mCursorEnd = ++mCursorStart;
    return true;
}

bool DebugConsole::Execute(const std::string& command)
{
    // TODO: parsing arguments, etc.

    const auto it = mFunctions.find(command);
    if (it != mFunctions.end())
    {
        const Function& func = it->second;
        func();
        return true;
    }

    Log("Unrecognized command: '" + mTypedCommand + "'", 0xFFFFFFFF);
    return false;
}

void DebugConsole::Log(const std::string& string, uint32 color)
{
    LogLine log;
    log.color = color;
    log.line = string;
    mLoggedLines.push_back(log);
}

void DebugConsole::Render(Common::Window* window, RenderContext* context)
{
    if (!mVisible)
        return;

    uint32 width, height;
    window->GetSize(width, height);

    // background
    Recti backgroundRect(0, 0, width, height / 2);
    GuiRenderer::Get()->DrawQuad(context, backgroundRect, 0xE0000000);

    // separator
    Recti editBoxSeparatorRect(0, height / 2 - mEditBoxHeight - 1, width, height / 2 - mEditBoxHeight);
    GuiRenderer::Get()->DrawQuad(context, editBoxSeparatorRect, 0x80FFFFFF);

    // calculate cursor position on screen
    int cursorStartX, cursorEndX, textLines;
    mFont->GetTextSize(mTypedCommand.c_str(), cursorStartX, textLines, mCursorStart);
    mFont->GetTextSize(mTypedCommand.c_str(), cursorEndX, textLines, mCursorEnd);

    // selection
    Recti selectionRect(mMargin + cursorStartX, height / 2 - mMargin,
                      mMargin + cursorEndX, height / 2 - mEditBoxHeight + mMargin);
    GuiRenderer::Get()->DrawQuad(context, selectionRect, 0x80FF8000);

    // selection
    Recti cursorRect(mMargin + cursorEndX, height / 2 - mMargin +2,
                     mMargin + cursorEndX + 1, height / 2 - mEditBoxHeight + mMargin - 2);
    GuiRenderer::Get()->DrawQuad(context, cursorRect, 0xFFFFFFFF);

    // edit box
    Recti editBoxRect(0, height / 2 - mEditBoxHeight, width, height / 2);
    editBoxRect.Reduce(mMargin);
    GuiRenderer::Get()->PrintText(context, mFont.get(), mTypedCommand.c_str(), editBoxRect, 0xFFFFFFFF,
        VerticalAlignment::Bottom, HorizontalAlignment::Left);


    const int spacing = mFontSize * 3 / 2;
    int yOffset = height / 2 - mEditBoxHeight;
    for (size_t i = mLoggedLines.size(); i-- > 0; )
    {
        Recti logRect(0, yOffset - spacing, width, yOffset);
        logRect.Reduce(mMargin);

        GuiRenderer::Get()->PrintText(context, mFont.get(), mLoggedLines[i].line.c_str(), logRect,
                                      mLoggedLines[i].color, VerticalAlignment::Bottom, HorizontalAlignment::Left);

        yOffset -= spacing;

        if (yOffset < 0)
            break;
    }
}

} // namespace Utils
} // namespace NFE
