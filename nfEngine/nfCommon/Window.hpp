/**
    NFEngine project

    \file   Window.hpp
    \brief  Window class declaration.
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
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    bool mClosed;
    HWND mHandle;
    HINSTANCE mInstance;

    BOOL mFullscreen;
    UINT mWidth;
    UINT mHeight;
    LONG mLeft;
    LONG mTop;

    std::wstring mTitle;
    wchar_t mWndClass[48];

    UINT mMouseButtons[3];
    int mMouseDownX[3];
    int mMouseDownY[3];

    BOOL mpKeys[256];

    // used by renderer
    WindowResizeCallback mResizeCallback;
    void* mResizeCallbackUserData;

    void lostFocus();
    void mouseDown(UINT Button, int X, int Y);
    void mouseUp(UINT Button);
    void mouseMove(int X, int Y);

    Window(const Window&);
    Window& operator= (const Window&);

public:
    Window();
    ~Window();

    bool open();
    bool close();

    HWND getHandle();
    void getSize(UINT& Width, UINT& Height);
    float getAspectRatio();
    BOOL getFullscreenMode();

    BOOL isMouseButtonDown(UINT Button);
    BOOL isKeyPressed(UCHAR Key);

    void setSize(UINT Width, UINT Height);
    void setFullscreenMode(BOOL Enabled);
    void setTitle(const wchar_t* pTitle);

    // WARINING: only engine shold call this function
    void setResizeCallback(WindowResizeCallback pFunc, void* pUserData);

    void processMessages();
    bool isClosed();

    //callbacks
    virtual void OnClose();
    virtual void OnResize(UINT width, UINT height);
    virtual void OnKeyPress(int key);
    virtual void OnScroll(int delta);
    virtual void OnMouseDown(UINT button, int x, int y);
    virtual void OnMouseMove(int x, int y, int deltaX, int deltaY);
    virtual void OnMouseUp(UINT button);
};

} // namespace Common
} // namespace NFE
