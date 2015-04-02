/**
 * @file   Window.cpp
 * @author mkkulagowski (mkkulagowski@gmail.com)
 * @brief  Window class definition.
 */

#include "stdafx.hpp"
#include "../Window.hpp"


namespace NFE {
namespace Common {

Window::Window()
{
    mWindow = 0;
    mClosed = true;
    mWidth = 200;
    mHeight = 200;
    mFullscreen = false;
    mResizeCallback = nullptr;
    mResizeCallbackUserData = nullptr;
    mTitle = "Window";

    if (!mDisplay)
    {
        mDisplay = XOpenDisplay(nullptr);
        if (mDisplay == nullptr)
        {
            printf("Cannot connect to X server\n");
            exit(1);
        }
    }
    mRoot = DefaultRootWindow(mDisplay);
    XSetScreenSaver(mDisplay, 0, 0, DontPreferBlanking, AllowExposures);

    for (int i = 0; i < 3; i++)
        mMouseButtons[i] = false;

    for (int i = 0; i < 255; i++)
        mKeys[i] = false;
}

Window::~Window()
{
    Close();
    XSetScreenSaver(mDisplay, -1, 0, DontPreferBlanking, AllowExposures);
    XDestroyWindow(mDisplay, mWindow);
    XCloseDisplay(mDisplay);
}

void Window::SetSize(uint32 width, uint32 height)
{
    mWidth = width;
    mHeight = height;

    if (!mClosed)
        XResizeWindow(mDisplay, mWindow, mWidth, mHeight);
}

void Window::SetTitle(const char* title)
{
    mTitle = title;
    if (!mClosed)
        XStoreName(mDisplay, mWindow, mTitle.c_str());
}

void Window::SetFullscreenMode(bool enabled)
{
    if (!mClosed)
    {
        if (mFullscreen && !enabled)
        {
            XEvent event;
            ::Atom wmStateAtom = XInternAtom(mDisplay, "_NET_WM_STATE", False);
            ::Atom fullscreenAtom = XInternAtom(mDisplay, "_NET_WM_STATE_FULLSCREEN", False);

            memset(&event, 0, sizeof(event));
            event.type = ClientMessage;
            event.xclient.window = mWindow;
            event.xclient.message_type = wmStateAtom;
            event.xclient.format = 32;
            event.xclient.data.l[0] = 0; // _NET_WM_STATE_REMOVE
            event.xclient.data.l[1] = fullscreenAtom;
            event.xclient.data.l[2] = 0;

            XSendEvent(mDisplay, mRoot, False, SubstructureNotifyMask, &event);
        }
        else if (!mFullscreen && enabled)
        {
            // enter fullscreen
            XEvent event;
            ::Atom wmStateAtom = XInternAtom(mDisplay, "_NET_WM_STATE", False);
            ::Atom fullscreenAtom = XInternAtom(mDisplay, "_NET_WM_STATE_FULLSCREEN", False);

            memset(&event, 0, sizeof(event));
            event.type = ClientMessage;
            event.xclient.window = mWindow;
            event.xclient.message_type = wmStateAtom;
            event.xclient.format = 32;
            event.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
            event.xclient.data.l[1] = fullscreenAtom;
            event.xclient.data.l[2] = 0;

            XSendEvent(mDisplay, mRoot, False, SubstructureNotifyMask, &event);
        }

        mFullscreen = enabled;

        if (mResizeCallback)
        {
            mResizeCallback(mResizeCallbackUserData);
        }
    }
}

bool Window::Open()
{
    if (!mClosed)
        return false;

    XSetWindowAttributes xSetWAttrib;
    ::Visual* visual = DefaultVisual(mDisplay, XDefaultScreen(mDisplay));
    ::Colormap colormap = XCreateColormap(mDisplay, mRoot, visual, AllocNone);
    xSetWAttrib.colormap = colormap;
    xSetWAttrib.event_mask = Button1MotionMask | Button2MotionMask | ButtonPressMask |
                            ButtonReleaseMask | ExposureMask | FocusChangeMask | KeyPressMask |
                            KeyReleaseMask | PointerMotionMask | StructureNotifyMask;

    XSetErrorHandler(ErrorHandler);
    mWindow = XCreateWindow(mDisplay, mRoot, 0, 0, mWidth, mHeight, 1, CopyFromParent,
                            InputOutput, CopyFromParent, CWColormap | CWEventMask, &xSetWAttrib);
    if (Window::mWindowError)
    {
        Window::mWindowError = false;
        return false;
    }
    XSetErrorHandler(nullptr);

    XStoreName(mDisplay, mWindow, mTitle.c_str());
    ::Atom WmDelete = XInternAtom(mDisplay, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(mDisplay, mWindow, &WmDelete, 1);
    XMapWindow(mDisplay, mWindow);
    mClosed = false;
    return true;
}

bool Window::Close()
{
    if (mClosed)
        return false;

    XUnmapWindow(mDisplay, mWindow);
    mClosed = true;
    return true;
}

void Window::MouseDown(uint32 button, int x, int y)
{
    XGrabPointer(mDisplay, mWindow, True, 0, GrabModeAsync,
                GrabModeAsync, mWindow, None, CurrentTime);
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
        XUngrabPointer(mDisplay, CurrentTime);

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

void Window::ProcessMessages()
{
    XEvent event;
    XNextEvent(mDisplay, &event);
    switch (event.type)
    {
        case ClientMessage:
        {
            if (event.xclient.data.l[0] == XInternAtom(mDisplay, "WM_DELETE_WINDOW", false))
                {
                    this->Close();
                    this->OnClose();
                }
            break;
        }
        case KeyPress:
        {
            mKeys[event.xkey.keycode] = true;
            OnKeyPress(event.xkey.keycode);
            break;
        }
        case KeyRelease:
        {
            this->mKeys[event.xkey.keycode] = false;
            break;
        }
        case MotionNotify:
        {
            this->MouseMove(event.xmotion.x, event.xmotion.y);
            break;
        }
        case ButtonPress:
        {
            if (event.xbutton.button < 4) // 1-3 MBtns, 4-5 MWheel
                this->MouseDown(event.xbutton.button - 1, event.xbutton.x, event.xbutton.y); //can be event.x_root,y_root
            else if (event.xbutton.button == 4)
                this->OnScroll(1); // btn==4 is UP,
            else
                this->OnScroll(-1); // btn==5 is DOWN
            break;
        }
        case ButtonRelease:
        {
            this->MouseUp(event.xbutton.button - 1);
            break;
        }
        case FocusOut:
        {
            this->LostFocus();
            break;
        }
        case ConfigureNotify:
        {
            XConfigureEvent confEvent = event.xconfigure;

            if (confEvent.width != mWidth || confEvent.height != mHeight)
            {
                mWidth = confEvent.width;
                mHeight = confEvent.height;
                this->OnResize(mWidth, mHeight);

                if (mResizeCallback)
                    mResizeCallback(mResizeCallbackUserData);
            }
            break;
        }
    }
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

void* Window::GetHandle() const
{
    return const_cast<void*>(static_cast<const void*>(&mWindow));
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

int Window::ErrorHandler(::Display *dpy, XErrorEvent *error)
{
    char errorCode[1024];
    XGetErrorText(dpy, error->error_code, errorCode, 1024);
    printf("_X Error of failed request: %s\n", errorCode);
    Window::mWindowError = true;
    return 0;
}

} // namespace Common
} // namespace NFE
