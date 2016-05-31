/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Window class declaration.
 */

#pragma once

#include "nfCommon.hpp"
#include "KeyCodes.hpp"

#if defined(__LINUX__) | defined(__linux__)
#include <X11/Xlib.h>
#endif // defined(__LINUX__) | defined(__linux__)

#define NFE_WINDOW_KEYS_NUM 256

namespace NFE {
namespace Common {

typedef void (*WindowResizeCallback)(void*);

/**
 * Simple UI windows class.
 */
class NFCOMMON_API Window
{
private:
#if defined(WIN32)
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    HWND mHandle;
    HINSTANCE mInstance;
    int mLeft;
    int mTop;
    wchar_t mWndClass[48];
#elif defined(__LINUX__) | defined(__linux__)
    ::Display* mDisplay; // TODO make only one X connection for all Window instances
    ::Window mWindow;
    ::Window mRoot;
    static bool mWindowError;
    static int ErrorHandler(::Display* dpy, XErrorEvent *error);
#else //...
#error "Target not supported!" // TODO Consider supporting Wayland as well
#endif // defined(WIN32)

    bool mClosed;
    bool mFullscreen;
    bool mInvisible;
    uint32 mWidth;
    uint32 mHeight;
    std::string mTitle;

    /// input recorded since last @p ProcessMessages() method call
    bool mMouseButtons[3];
    int mMousePos[2];
    int mMouseWheelDelta;
    std::string mCharacters;
    bool mKeys[NFE_WINDOW_KEYS_NUM];

    // used by renderer
    WindowResizeCallback mResizeCallback;
    void* mResizeCallbackUserData;

    void LostFocus();
    void MouseDown(uint32 button, int x, int y);
    void MouseUp(uint32 button);
    void MouseMove(int x, int y);

    Window(const Window&);
    Window& operator= (const Window&);

public:
    Window();
    ~Window();

    bool Open();
    bool Close();

    void* GetHandle() const;
    void GetSize(uint32& width, uint32& height) const;
    float GetAspectRatio() const;
    bool GetFullscreenMode() const;


    NFE_INLINE int GetMouseWheelDelta() const
    {
        return mMouseWheelDelta;
    }

    NFE_INLINE void GetMousePosition(int& x, int& y) const
    {
        x = mMousePos[0];
        y = mMousePos[1];
    }

    NFE_INLINE bool IsMouseButtonDown(uint32 button) const
    {
        return mMouseButtons[button];
    }

    /**
     * Check if a key is pressed.
     */
    NFE_INLINE bool IsKeyPressed(KeyCode key) const
    {
        unsigned int k = static_cast<unsigned int>(key);
        if (k <= NFE_WINDOW_KEYS_NUM)
            return mKeys[k];
        return false;
    }

    /**
     * Get typed UTF-8 characters typed since last @p ProcessMessages() method call.
     */
    NFE_INLINE const char* GetInputCharacters() const
    {
        return mCharacters.c_str();
    }

    void SetSize(uint32 width, uint32 height);
    void SetFullscreenMode(bool enabled);
    void SetInvisible(bool invisible);
    void SetTitle(const char* title);

    // WARINING: only engine should call this function
    void SetResizeCallback(WindowResizeCallback func, void* userData);

    void ProcessMessages();
    bool IsClosed() const;
    bool HasFocus() const;

    // callbacks
    virtual void OnClose();
    virtual void OnResize(uint32 width, uint32 height);
    virtual void OnKeyPress(KeyCode key);
    virtual void OnScroll(int delta);
    virtual void OnMouseDown(uint32 button, int x, int y);
    virtual void OnMouseMove(int x, int y, int deltaX, int deltaY);
    virtual void OnMouseUp(uint32 button);
};

} // namespace Common
} // namespace NFE
