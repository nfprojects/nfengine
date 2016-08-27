/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Simple keyboard input handling declarations
 */

#pragma once

#include "../Core.hpp"
#include "nfCommon/nfCommon.hpp"
#include "nfCommon/KeyCodes.hpp"

namespace NFE {
namespace Utils {

// keyboard key press event
struct KeyPressedEvent
{
    Common::KeyCode key;

    bool isCtrlPressed : 1;
    bool isShiftPressed : 1;
    bool isAltPressed : 1;
};

// mouse button down/up event
struct MouseButtonEvent
{
    int mouseButton;
    int32 x;
    int32 y;
};

struct MouseMoveEvent
{
    int mouseButton;
    int32 x;
    int32 y;
};

class CORE_API SimpleInputListener
{
public:
    virtual ~SimpleInputListener() { }
    NFE_INLINE virtual bool OnKeyPressed(const KeyPressedEvent& event) { return false; }
    NFE_INLINE virtual bool OnMouseDown(const MouseButtonEvent& event) { return false; }
    NFE_INLINE virtual bool OnMouseUp(const MouseButtonEvent& event) { return false; }
    NFE_INLINE virtual bool OnMouseMove(const MouseMoveEvent& event) { return false; }
    NFE_INLINE virtual bool OnMouseScroll(int delta) { return false; }
    NFE_INLINE virtual bool OnCharTyped(const char*) { return false; }
};

} // namespace Utils
} // namespace NFE
