/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Window class definition.
 */

#include "../PCH.hpp"
#include "Common.hpp"
#include "../Window.hpp"

namespace NFE {
namespace Common {

#define USE_ANT_TWEAK


#ifdef USE_ANT_TWEAK
#include "AntTweakBar/include/AntTweakBar.h"
#endif

namespace {

const DWORD gWindowedExStyle = WS_EX_WINDOWEDGE;
const DWORD gWindowedStyle = WS_OVERLAPPEDWINDOW;
const DWORD gFullscreenExStyle = 0;
const DWORD gFullscreenStyle = WS_POPUP | WS_SYSMENU;

} // namespace

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


Window::Window()
{
    mInstance = GetModuleHandle(0);
    mHandle = 0;
    mClosed = true;
    mWidth = 200;
    mHeight = 200;
    mLeft = 10;
    mTop = 10;
    mFullscreen = false;
    mResizeCallback = nullptr;
    mResizeCallbackUserData = nullptr;

    mTitle = "Window";

    swprintf_s(mWndClass, L"%ws_%p", L"nfEngine_WndClass", this);

    WNDCLASSEX wcex;
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = mInstance;
    wcex.hIcon         = 0; //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME));
    wcex.hIconSm       = 0; //LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = 0; //(HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName  = 0;
    wcex.lpszClassName = mWndClass;
    RegisterClassEx(&wcex);

    for (int i = 0; i < 3; i++)
        mMouseButtons[i] = false;

    for (int i = 0; i < 255; i++)
        mKeys[i] = false;
}

Window::~Window()
{
    Close();
    UnregisterClass(mWndClass, mInstance);
}

void Window::SetSize(uint32 width, uint32 height)
{
    mWidth = width;
    mHeight = height;

    if (!mClosed)
        MoveWindow(mHandle, mLeft, mTop, width, height, TRUE);
}

void Window::SetTitle(const char* title)
{
    mTitle = title;
    if (!mClosed)
    {
        std::wstring wideTitle;
        if (UTF8ToUTF16(title, wideTitle))
            SetWindowText(mHandle, wideTitle.c_str());
    }
}

void Window::SetFullscreenMode(bool enabled)
{
    if (mFullscreen && !enabled)
    {
        // escape fullscreen
        RECT windowRect;
        windowRect.left = mLeft;
        windowRect.right = mLeft + mWidth;
        windowRect.top = mTop;
        windowRect.bottom = mTop + mHeight;
        AdjustWindowRectEx(&windowRect, gWindowedStyle, FALSE, gWindowedExStyle);

        SetWindowLong(mHandle, GWL_EXSTYLE, gWindowedExStyle);
        SetWindowLong(mHandle, GWL_STYLE, gWindowedStyle);
        SetWindowPos(mHandle, HWND_NOTOPMOST,
                     mTop, mTop,
                     windowRect.right - windowRect.left,
                     windowRect.bottom - windowRect.top,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
    else if (!mFullscreen && enabled)
    {
        // enter fullscreen
        SetWindowLong(mHandle, GWL_EXSTYLE, gFullscreenExStyle);
        SetWindowLong(mHandle, GWL_STYLE, WS_VISIBLE | WS_POPUP );
        SetWindowPos(mHandle, nullptr, 0, 0, mWidth, mHeight, SWP_NOZORDER );
    }

    mFullscreen = enabled;

    if (mResizeCallback)
    {
        mResizeCallback(mResizeCallbackUserData);
    }
}


bool Window::Open()
{
    if (!mClosed)
        return false;
    std::wstring wideTitle;
    if(!UTF8ToUTF16(mTitle, wideTitle))
        return false;
    if (mFullscreen)
    {
        mHandle = CreateWindowEx(gFullscreenExStyle, mWndClass, wideTitle.c_str(),
                                 gFullscreenStyle, 0, 0, mWidth, mHeight,
                                 nullptr, nullptr, mInstance, nullptr);
    }
    else
    {
        RECT windowRect;
        windowRect.left = (long)mLeft;
        windowRect.right = (long)(mWidth + mLeft);
        windowRect.top = (long)mTop;
        windowRect.bottom = (long)(mHeight + mTop);
        AdjustWindowRectEx(&windowRect, gWindowedStyle, FALSE, gWindowedExStyle);

        mLeft = -windowRect.left;
        mTop = -windowRect.top;

        mHandle = CreateWindowEx(gWindowedExStyle,
                                 mWndClass, wideTitle.c_str(),
                                 gWindowedStyle,
                                 mTop, mTop,
                                 windowRect.right - windowRect.left,
                                 windowRect.bottom - windowRect.top,
                                 nullptr, nullptr, mInstance, nullptr);
    }

    if (!mHandle)
        return false;

    SetWindowLongPtr(mHandle, GWLP_USERDATA, (LONG_PTR)this);
    SetWindowText(mHandle, wideTitle.c_str());
    ShowWindow(mHandle, SW_SHOW);
    UpdateWindow(mHandle);
    SetFocus(mHandle);

    mClosed = false;
    return true;
}

bool Window::Close()
{
    if (mClosed)
        return false;

    ShowWindow(mHandle, SW_HIDE);
    mClosed = true;
    return true;
}


void Window::MouseDown(uint32 button, int x, int y)
{
    SetCapture(mHandle);
    mMouseButtons[button] = true;
    mMouseDownX[button] = x;
    mMouseDownY[button] = y;

    OnMouseDown(button, x, y);
}

void Window::MouseUp(uint32 button)
{
    mMouseButtons[button] = false;

    bool ButtonsReleased = true;
    for (int i = 0; i < 3; i++)
        if (mMouseButtons[i])
            ButtonsReleased = false;

    if (ButtonsReleased)
        ReleaseCapture();

    OnMouseUp(button);
}

void Window::MouseMove(int x, int y)
{
    OnMouseMove(x, y, x - mMouseDownX[0], y - mMouseDownY[0]);
    mMouseDownX[0] = x;
    mMouseDownY[0] = y;
}

bool Window::IsKeyPressed(int Key) const
{
    return mKeys[Key];
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window* window = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (!window)
        return DefWindowProc(hWnd, message, wParam, lParam);

#ifdef USE_ANT_TWEAK
    if (window->HasFocus())
    {
        bool handleByTweakBar = true;

        for (int i = 0; i < 3; i++)
            if (window->mMouseButtons[i])
                handleByTweakBar = false;

        if (handleByTweakBar)
            if (TwEventWin(hWnd, message, wParam, lParam))
                return 0;
    }
#endif

    switch (message)
    {
        case WM_SYSCOMMAND:
        {
            switch (wParam)
            {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                    return 0;
            }
            break;
        }

        case WM_CLOSE:
        {
            window->Close();
            window->OnClose();
            return 0;
        }

        case WM_SIZE:
        {
            if (wParam != SIZE_MINIMIZED)
            {
                window->mWidth = LOWORD(lParam);
                window->mHeight = HIWORD(lParam);
                window->OnResize(window->mWidth, window->mHeight);

                if (window->mResizeCallback)
                    window->mResizeCallback(window->mResizeCallbackUserData);
            }

            return 0;
        }

        case WM_KEYDOWN:
        {
            window->mKeys[wParam] = true;
            window->OnKeyPress((int)wParam);
            return 0;
        }

        case WM_KEYUP:
        {
            window->mKeys[wParam] = false;
            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            window->OnScroll(GET_WHEEL_DELTA_WPARAM(wParam));
            return 0;
        }


        // MOUSE
        case WM_LBUTTONDOWN:
        {
            window->MouseDown(0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_LBUTTONUP:
        {
            window->MouseUp(0);
            return 0;
        }


        case WM_MBUTTONDOWN:
        {
            window->MouseDown(2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_MBUTTONUP:
        {
            window->MouseUp(2);
            return 0;
        }


        case WM_RBUTTONDOWN:
        {
            window->MouseDown(1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_RBUTTONUP:
        {
            window->MouseUp(1);
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            window->MouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_ACTIVATE:
        {
            if (wParam == WA_INACTIVE)
                window->LostFocus();
            return 0;
        }

    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void Window::LostFocus()
{
    MouseUp(0);
    MouseUp(1);
    MouseUp(2);

    for (int i = 0; i < 256; i++)
        mKeys[i] = false;
}

bool Window::IsClosed() const
{
    return mClosed;
}

bool Window::HasFocus() const
{
    return GetActiveWindow() == mHandle;
}

void* Window::GetHandle() const
{
    return static_cast<void*>(mHandle);
}

void Window::GetSize(uint32& Width, uint32& Height) const
{
    Width = mWidth;
    Height = mHeight;
}

float Window::GetAspectRatio() const
{
    return (float)mWidth / (float)mHeight;
}

bool Window::GetFullscreenMode() const
{
    return mFullscreen;
}

bool Window::IsMouseButtonDown(uint32 button) const
{
    return mMouseButtons[button];
}


void Window::ProcessMessages()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            Close();
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void Window::SetResizeCallback(WindowResizeCallback func, void* userData)
{
    mResizeCallback = func;
    mResizeCallbackUserData = userData;
}

void Window::OnClose()
{
}

void Window::OnResize(uint32 width, uint32 height)
{
    (void)width;
    (void)height;
}

void Window::OnKeyPress(int key)
{
    (void)key;
}

void Window::OnScroll(int delta)
{
    (void)delta;
}

void Window::OnMouseDown(uint32 button, int x, int y)
{
    (void)button;
    (void)x;
    (void)y;
}

void Window::OnMouseMove(int x, int y, int deltaX, int deltaY)
{
    (void)x;
    (void)y;
    (void)deltaX;
    (void)deltaY;
}

void Window::OnMouseUp(uint32 button)
{
    (void)button;
}

} // namespace Common
} // namespace NFE
