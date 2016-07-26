/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Window class definition.
 */

#include "../PCH.hpp"
#include "../Window.hpp"
#include "Common.hpp"

#include <Windowsx.h>


namespace NFE {
namespace Common {

namespace {

const DWORD gWindowedExStyle = WS_EX_WINDOWEDGE;
const DWORD gWindowedStyle = WS_OVERLAPPEDWINDOW;
const DWORD gFullscreenExStyle = 0;
const DWORD gFullscreenStyle = WS_POPUP | WS_SYSMENU;

WPARAM MapLeftRightSpecialKey(WPARAM wParam, LPARAM lParam)
{
    WPARAM newKey;

    // 16-23 bits are scancode needed for left/right distinguishment
    UINT scanCode = (lParam & 0x00FF0000) >> 16;
    // 24th bit is an "extended" bit, set to true if right control/alt are pressed
    UINT extended = (lParam & 0x01000000);

    switch (wParam)
    {
    case VK_SHIFT:
        newKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
        if (newKey == 0)
            // MapVirtualKey failed to map scan code to vkey, fallback to old value
            newKey = wParam;
        break;
    case VK_CONTROL:
        newKey = extended ? VK_RCONTROL : VK_LCONTROL;
        break;
    case VK_MENU:
        newKey = extended ? VK_RMENU : VK_LMENU;
        break;
    default:
        return wParam;
    }

    return newKey;
}

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
    mInvisible = false;
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

    mMouseWheelDelta = 0;
    for (int i = 0; i < 3; i++)
        mMouseButtons[i] = false;

    for (int i = 0; i < NFE_WINDOW_KEYS_NUM; i++)
        mKeys[i] = false;

    mMousePos[0] = mMousePos[1] = -1;
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

void Window::SetInvisible(bool invisible)
{
    mInvisible = invisible;

    if (mHandle)
    {
        if (mInvisible)
            ShowWindow(mHandle, SW_SHOW);
        else
            ShowWindow(mHandle, SW_HIDE);
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

    if (!mInvisible)
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

    if (!mInvisible)
        ShowWindow(mHandle, SW_HIDE);

    DestroyWindow(mHandle);
    mClosed = true;
    return true;
}


void Window::MouseDown(uint32 button, int x, int y)
{
    SetCapture(mHandle);
    mMouseButtons[button] = true;
    mMousePos[0] = x;
    mMousePos[1] = y;

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
    OnMouseMove(x, y, x - mMousePos[0], y - mMousePos[1]);
    mMousePos[0] = x;
    mMousePos[1] = y;
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window* window = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (!window)
        return DefWindowProc(hWnd, message, wParam, lParam);

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
            wParam = MapLeftRightSpecialKey(wParam, lParam);
            window->mKeys[wParam] = true;
            window->OnKeyPress(static_cast<KeyCode>(wParam));
            return 0;
        }

        case WM_KEYUP:
        {
            wParam = MapLeftRightSpecialKey(wParam, lParam);
            window->mKeys[wParam] = false;
            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            window->mMouseWheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            window->OnScroll(window->mMouseWheelDelta);
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

        case WM_CHAR:
        {
            wchar_t input[] = { static_cast<wchar_t>(wParam), 0 };
            const int bufferSize = 8;
            char buffer[bufferSize];

            // convert UTF-16 char to UTF-8
            int result = ::WideCharToMultiByte(CP_UTF8, 0, input, 1, buffer, bufferSize, 0, 0);
            if (result > 0)
            {
                buffer[result] = '\0';
                window->mCharacters.append(buffer);
            }

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

    for (int i = 0; i < NFE_WINDOW_KEYS_NUM; i++)
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

void Window::ProcessMessages()
{
    mCharacters.clear();

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

void Window::OnKeyPress(KeyCode key)
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