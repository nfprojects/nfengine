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

DebugConsole::DebugConsole()
    : mVisible(false)
    , mCursorStart(0)
    , mCursorEnd(0)
{
    mMargin = 6;
    mEditBoxHeight = 24;

    mFont.reset(new Font);
    mFont->Init("nfEngineDemo/Data/Fonts/Inconsolata.otf", 11);
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
        Log log;
        log.color = 0xFFD0D0D0;
        log.line = mTypedCommand;
        mLogMessages.push_back(log);

        mCommandsHistory.push_back(mTypedCommand);

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

void DebugConsole::Render(Common::Window* window, RenderContext* context)
{
    if (!mVisible)
        return;

    uint32 width, height;
    window->GetSize(width, height);

    // background
    Recti backgroundRect(0, 0, width, height / 2);
    GuiRenderer::Get()->DrawQuad(context, backgroundRect, 0xC0000000);

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

    /*
    for (size_t i = mLogMessages.size(); i-- > 0; )
    {
        Recti editBoxRect(0, height / 2 - mEditBoxHeight, width, height / 2);
        editBoxRect.Reduce(mMargin);
    }
    */
}

} // namespace Utils
} // namespace NFE
