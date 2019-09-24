/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Window class declaration.
 */

#pragma once

#include "../nfCommon.hpp"
#include "KeyCodes.hpp"
#include "../Containers/String.hpp"

#if defined(__LINUX__) | defined(__linux__)
#include <xcb/xcb.h>
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
    HDC mDC;
    HINSTANCE mInstance;
    wchar_t mWndClass[48];
#elif defined(__LINUX__) | defined(__linux__)
    xcb_connection_t* mConnection;
    xcb_window_t mWindow;
    xcb_screen_t* mScreen;
    xcb_intern_atom_reply_t* mDeleteReply;
    int mConnScreen;
#else
#error "Target not supported!" // TODO Consider supporting Wayland as well
#endif // defined(WIN32)

    bool mClosed;
    bool mFullscreen;
    bool mInvisible;
    uint32 mWidth;
    uint32 mHeight;
    int mLeft;
    int mTop;
    String mTitle;

    /// input recorded since last @p ProcessMessages() method call
    bool mMouseButtons[3];
    int mMousePos[2];
    int mMouseWheelDelta;
    bool mKeys[NFE_WINDOW_KEYS_NUM];

    // used by renderer
    WindowResizeCallback mResizeCallback;
    void* mResizeCallbackUserData;

    void LostFocus();
    void MouseDown(MouseButton button, int x, int y);
    void MouseUp(MouseButton button);
    void MouseMove(int x, int y);

    Window(const Window&);
    Window& operator= (const Window&);

public:
    Window();
    ~Window();

    /**
     * Initializes Window connection to the system.
     *
     * @return True on success, false on failure
     */
    bool Init();

    /**
     * Creates and opens a Window, if it isn't opened already.
     *
     * @return True on success, false on failure
     */
    bool Open();

    /**
     * Closes a Window which is already opened.
     *
     * @return True on success, false on failure
     *
     * @remarks This function will free any Window-related resources on the system. Should
     * the window still be valid, but invisible for the user, a @p SetInvisible function
     * should be used.
     */
    bool Close();

    /**
     * Acquire system-specific Window handle
     */
    void* GetHandle() const;

    /**
     * Get Window's size
     */
    void GetSize(uint32& width, uint32& height) const;

    /**
     * Get Window's current aspect ratio.
     */
    float GetAspectRatio() const;

    /**
     * Check if window is currently in Fullscreen mode.
     */
    bool GetFullscreenMode() const;

    /**
     * Acquire how much did the mouse wheel scrool.
     */
    NFE_INLINE int GetMouseWheelDelta() const
    {
        return mMouseWheelDelta;
    }

    /**
     * Acquire current mouse position relative to window.
     */
    NFE_INLINE void GetMousePosition(int& x, int& y) const
    {
        x = mMousePos[0];
        y = mMousePos[1];
    }

    /**
     * Check if a mouse button is pressed
     */
    NFE_INLINE bool IsMouseButtonDown(MouseButton button) const
    {
        return mMouseButtons[static_cast<uint32>(button)];
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
     * Set window's new size
     */
    void SetSize(uint32 width, uint32 height);

    /**
     * Enable or disable fullscreen
     */
    void SetFullscreenMode(bool enabled);

    /**
     * Make window invisible or visible
     */
    void SetInvisible(bool invisible);

    /**
     * Set window's title
     */
    void SetTitle(const char* title);

    // Display image in the window
    // Image must be in 32-bit BGRA format
    // TODO: accept Bitmap instead
    bool DrawPixels(const uint8* data, uint32 width, uint32 height, uint32 stride);

    /**
     * Set callback to be used when Window is resized.
     *
     * @param func Pointer to function called when resize occurs.
     * @param userData User data to be attached each time @p func is called.
     *
     * @remarks In this project only engine should call this function
     */
    void SetResizeCallback(WindowResizeCallback func, void* userData);

    /**
     * Process messages from Window system.
     */
    void ProcessMessages();

    /**
     * Is the window closed?
     */
    bool IsClosed() const;

    /**
     * Is the window focused on the system?
     */
    bool HasFocus() const;

    // callbacks, overridden by inheritance
    virtual void OnClose();
    virtual void OnResize(uint32 width, uint32 height);
    virtual void OnKeyPress(KeyCode key);
    virtual void OnKeyUp(KeyCode key);
    virtual void OnCharTyped(const char* charUTF8);
    virtual void OnScroll(int delta);
    virtual void OnMouseDown(MouseButton button, int x, int y);
    virtual void OnMouseMove(int x, int y, int deltaX, int deltaY);
    virtual void OnMouseUp(MouseButton button);
    virtual void OnFileDrop(const String& filePath);
};

} // namespace Common
} // namespace NFE
