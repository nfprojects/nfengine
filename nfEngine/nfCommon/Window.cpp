/**
 * @file   Window.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Window class definition.
 */

#include "stdafx.hpp"
#include "Window.hpp"

namespace NFE {
namespace Common {

#define USE_ANT_TWEAK


#ifdef USE_ANT_TWEAK
#include "AntTweekBar/include/AntTweakBar.h"
#endif

const DWORD WindowedExStyle = WS_EX_WINDOWEDGE;
const DWORD WindowedStyle =
    WS_OVERLAPPEDWINDOW; //WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU  | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
const DWORD FullscreenExStyle = 0;
const DWORD FullscreenStyle = WS_POPUP | WS_SYSMENU;

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
    mResizeCallback = NULL;
    mResizeCallbackUserData = NULL;

    mTitle = L"Window";


    swprintf_s(mWndClass, L"%ws_%p", L"nfEngine_WndClass", this);
    //mWndClass = L"nfEngine_WndClass";;

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = mInstance;
    wcex.hIcon          = 0; //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME));
    wcex.hIconSm        = 0; //LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = 0; //(HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = mWndClass;
    RegisterClassEx(&wcex);

    for (int i = 0; i < 3; i++)
        mMouseButtons[i] = 0;

    for (int i = 0; i < 255; i++)
        mpKeys[i] = 0;
}

Window::~Window()
{
    close();
    UnregisterClass(mWndClass, mInstance);
}

void Window::setSize(UINT Width, UINT Height)
{
    mWidth = Width;
    mHeight = Height;

    if (!mClosed)
        MoveWindow(mHandle, mLeft, mTop, Width, Height, TRUE);
}

void Window::setTitle(const wchar_t* pTitle)
{
    mTitle = pTitle;
    if (!mClosed)
        SetWindowText(mHandle, mTitle.c_str());
}

void Window::setFullscreenMode(BOOL Enabled)
{
    if (mFullscreen && !Enabled)
    {
        //escape fullscreen
        RECT WindowRect;
        WindowRect.left = mLeft;
        WindowRect.right = mLeft + mWidth;
        WindowRect.top = mTop;
        WindowRect.bottom = mTop + mHeight;
        AdjustWindowRectEx(&WindowRect, WindowedStyle, FALSE, WindowedExStyle);

        SetWindowLong(mHandle, GWL_EXSTYLE, WindowedExStyle);
        SetWindowLong(mHandle, GWL_STYLE, WindowedStyle);
        SetWindowPos(mHandle, HWND_NOTOPMOST,
                     mTop, mTop,
                     WindowRect.right - WindowRect.left,
                     WindowRect.bottom - WindowRect.top,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
    else if (!mFullscreen && Enabled)
    {
        //enter fullscreen
        SetWindowLong(mHandle, GWL_EXSTYLE, FullscreenExStyle);
        SetWindowLong(mHandle, GWL_STYLE, WS_VISIBLE | WS_POPUP );
        SetWindowPos(mHandle, NULL, 0, 0, mWidth, mHeight, SWP_NOZORDER );
    }

    mFullscreen = Enabled;

    if (mResizeCallback)
    {
        mResizeCallback(mResizeCallbackUserData);
    }
}


bool Window::open()
{
    if (!mClosed)
        return false;

    if (mFullscreen)
    {
        mHandle = CreateWindowEx(FullscreenExStyle, mWndClass, mTitle.c_str(),
                                 FullscreenStyle, 0, 0, mWidth, mHeight,
                                 NULL, NULL, mInstance, NULL);
    }
    else
    {
        RECT WindowRect;
        WindowRect.left = (long)mLeft;
        WindowRect.right = (long)(mWidth + mLeft);
        WindowRect.top = (long)mTop;
        WindowRect.bottom = (long)(mHeight + mTop);
        AdjustWindowRectEx(&WindowRect, WindowedStyle, FALSE, WindowedExStyle);

        mLeft = -WindowRect.left;
        mTop = -WindowRect.top;

        mHandle = CreateWindowEx(WindowedExStyle,
                                 mWndClass, mTitle.c_str(),
                                 WindowedStyle,
                                 mTop, mTop,
                                 WindowRect.right - WindowRect.left,
                                 WindowRect.bottom - WindowRect.top,
                                 NULL, NULL, mInstance, NULL);
    }

    if (!mHandle)
        return false;

    SetWindowLongPtr(mHandle, GWLP_USERDATA, (LONG_PTR)this);
    SetWindowText(mHandle, mTitle.c_str());
    ShowWindow(mHandle, SW_SHOW);
    UpdateWindow(mHandle);
    SetFocus(mHandle);

    mClosed = false;
    return true;
}

bool Window::close()
{
    if (mClosed)
        return false;

    ShowWindow(mHandle, SW_HIDE);
    mClosed = true;
    return true;
}


void Window::mouseDown(UINT Button, int X, int Y)
{
    SetCapture(mHandle);
    mMouseButtons[Button] = 1;
    mMouseDownX[Button] = X;
    mMouseDownY[Button] = Y;

    OnMouseDown(Button, X, Y);
}

void Window::mouseUp(UINT Button)
{
    mMouseButtons[Button] = 0;

    bool ButtonsReleased = true;
    for (int i = 0; i < 3; i++)
        if (mMouseButtons[i])
            ButtonsReleased = false;

    if (ButtonsReleased)
        ReleaseCapture();

    OnMouseUp(Button);
}

void Window::mouseMove(int X, int Y)
{
    OnMouseMove(X, Y, X - mMouseDownX[0], Y - mMouseDownY[0]);
    mMouseDownX[0] = X;
    mMouseDownY[0] = Y;
}

BOOL Window::isKeyPressed(UCHAR Key)
{
    return mpKeys[Key];
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window* pWindow = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (!pWindow)
        return DefWindowProc(hWnd, message, wParam, lParam);

#ifdef USE_ANT_TWEAK
    bool handleByTweakBar = true;

    for (int i = 0; i < 3; i++)
        if (pWindow->mMouseButtons[i])
            handleByTweakBar = false;

    if (handleByTweakBar)
        if (TwEventWin(hWnd, message, wParam, lParam))
            return 0;
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
            pWindow->close();
            pWindow->OnClose();
            return 0;
        }

        case WM_SIZE:
        {
            if (wParam != SIZE_MINIMIZED)
            {
                pWindow->mWidth = LOWORD(lParam);
                pWindow->mHeight = HIWORD(lParam);
                pWindow->OnResize(pWindow->mWidth, pWindow->mHeight);

                if (pWindow->mResizeCallback)
                    pWindow->mResizeCallback(pWindow->mResizeCallbackUserData);
            }

            return 0;
        }

        case WM_KEYDOWN:
        {
            pWindow->mpKeys[wParam] = 1;
            pWindow->OnKeyPress((int)wParam);
            return 0;
        }

        case WM_KEYUP:
        {
            pWindow->mpKeys[wParam] = 0;
            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            pWindow->OnScroll(GET_WHEEL_DELTA_WPARAM(wParam));
            return 0;
        }


        // MOUSE
        case WM_LBUTTONDOWN:
        {
            pWindow->mouseDown(0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_LBUTTONUP:
        {
            pWindow->mouseUp(0);
            return 0;
        }


        case WM_MBUTTONDOWN:
        {
            pWindow->mouseDown(2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_MBUTTONUP:
        {
            pWindow->mouseUp(2);
            return 0;
        }


        case WM_RBUTTONDOWN:
        {
            pWindow->mouseDown(1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_RBUTTONUP:
        {
            pWindow->mouseUp(1);
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            pWindow->mouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_ACTIVATE:
        {
            if (wParam == WA_INACTIVE)
                pWindow->lostFocus();
            return 0;
        }

    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void Window::lostFocus()
{
    mouseUp(0);
    mouseUp(1);
    mouseUp(2);

    for (int i = 0; i < 256; i++)
        mpKeys[i] = 0;
}

bool Window::isClosed()
{
    return mClosed;
}

HWND Window::getHandle()
{
    return mHandle;
}

void Window::getSize(UINT& Width, UINT& Height)
{
    Width = mWidth;
    Height = mHeight;
}

float Window::getAspectRatio()
{
    return (float)mWidth / (float)mHeight;
}

BOOL Window::getFullscreenMode()
{
    return mFullscreen;
}

BOOL Window::isMouseButtonDown(UINT Button)
{
    return mMouseButtons[Button];
}


void Window::processMessages()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            close();
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void Window::setResizeCallback(WindowResizeCallback pFunc, void* pUserData)
{
    mResizeCallback = pFunc;
    mResizeCallbackUserData = pUserData;
}

void Window::OnClose()
{
}

void Window::OnResize(UINT width, UINT height)
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

void Window::OnMouseDown(UINT button, int x, int y)
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

void Window::OnMouseUp(UINT button)
{
    (void)button;
}

} // namespace Common
} // namespace NFE
