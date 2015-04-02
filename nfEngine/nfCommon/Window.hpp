/**
 * @file   Window.hpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Window class declaration.
 */



#pragma once
#include "nfCommon.hpp"


namespace NFE {
namespace Common {

typedef void (*WindowResizeCallback)(void*);

/**
 * Simple UI windows class.
 */
// TODO: make this class platform independent
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
#endif // defined(WIN32)

    bool mClosed;
    bool mFullscreen;
    uint32 mWidth;
    uint32 mHeight;

    std::string mTitle;

    bool mMouseButtons[3];
    int mMouseDownX[3];
    int mMouseDownY[3];

    bool mKeys[256];
 
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

    bool IsMouseButtonDown(uint32 button) const;
    bool IsKeyPressed(int key) const;

    void SetSize(uint32 hidth, uint32 height);
    void SetFullscreenMode(bool enabled);
    void SetTitle(const char* title);

    // WARINING: only engine should call this function
    void SetResizeCallback(WindowResizeCallback func, void* userData);

    void ProcessMessages();
    bool IsClosed() const;

    // callbacks
    virtual void OnClose();
    virtual void OnResize(uint32 width, uint32 height);
    virtual void OnKeyPress(int key);
    virtual void OnScroll(int delta);
    virtual void OnMouseDown(uint32 button, int x, int y);
    virtual void OnMouseMove(int x, int y, int deltaX, int deltaY);
    virtual void OnMouseUp(uint32 button);
};

} // namespace Common
} // namespace NFE
