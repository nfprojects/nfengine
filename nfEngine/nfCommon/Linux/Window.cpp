/**
 * @file   Window.cpp
 * @author mkkulagowski (mkkulagowski@gmail.com)
 * @brief  Window class definition.
 */

#include "stdafx.hpp"
#include "../Window.hpp"


namespace NFE {
namespace Common {

::Display Window::mDisplay = NULL;

Window::Window()
{
    mClosed = true;
    mWidth = 200;
    mHeight = 200;
    mFullscreen = false;
    mResizeCallback = NULL;
    mResizeCallbackUserData = NULL;
    mTitle = "Window";

    if (!mDisplay)
    {
        mDisplay = XOpenDisplay(NULL);
        if (mDisplay == NULL)
        {
            LOG_ERROR("Cannot connect to X server\n");
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

    /*XStoreName changes only name in titlebar. Name of an application
    (used maybe in some process mgr) is set by XSetClassHint(display, window, class_hints)
    like that:
        classHint = XAllocClassHint();
        if (classHint) {
            classHint->res_name = mTitle;
            classHint->res_class = "nfEngine";
        }
        XSetClassHint(display, window, classHint);
        XFree(classHint);
    */
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
            XFree(event);
        }
        else if (!mFullscreen && enabled)
        {
            // enter fullscreen
            //XMoveResizeWindow(display, window, 0, 0, XWAttrib.width, XWAttrib.height);
            XEvent event;
            Atom wmStateAtom = XInternAtom(mDisplay, "_NET_WM_STATE", False);
            Atom fullscreenAtom = XInternAtom(mDisplay, "_NET_WM_STATE_FULLSCREEN", False);

            memset(&event, 0, sizeof(event));
            event.type = ClientMessage;
            event.xclient.window = mWindow;
            event.xclient.message_type = wmStateAtom;
            event.xclient.format = 32;
            event.xclient.data.l[0] = 1; // _NET_WM_STATE_ADD
            event.xclient.data.l[1] = fullscreenAtom;
            event.xclient.data.l[2] = 0;

            XSendEvent(mDisplay, mRoot, False, SubstructureNotifyMask, &event);
            XFree(event);
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
    XVisualInfo visual = DefaultVisual(mDisplay, XGetDefaultScreen(mDisplay));
    ::Colormap colormap = XCreateColormap(mDisplay, mRoot, visual->visual, AllocNone);
    xSetWAttrib.colormap = colormap;
    xSetWAttrib.event_mask = Button1MotionMask | Button2MotionMask | ButtonPressMask |
                            ButtonReleaseMask | ExposureMask | FocusChangeMask | KeyPressMask |
                            KeyReleaseMask | PointerMotionMask | StructureNotifyMask;

    XSetErrorHandler(Window::ErrorHandler);
    mWindow = XCreateWindow(mDisplay, mRoot, 0, 0, mWidth, mHeight, 1, CopyFromParent,
                            InputOutput, CopyFromParent, CWColormap | CWEventMask, &xSetWAttrib);
    XSetErrorHandler(NULL);
    if (mWindowError)
    {
        mWindowError = false;
        return false;
    }

    XStoreName(mDisplay, mWindow, mTitle.c_str());
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
            if (event.xclient.message_type == wm_protocols &&
                event.xclient.data.l[0] == wm_delete_this)
                {
                    this->Close();
                    this->OnClose();
                }
            break;
        }
        case KeyPress:
        {
            mKeys[event.keycode] = true;
            OnKeyPress(event.keycode);
            break;
        }
        case KeyRelease:
        {
            this->mKeys[event.keycode] = false;
            break;
        }
        case MotionNotify:
        {
            this->MouseMove(event.x, event.y);
            break;
        }
        case ButtonPress:
        {
            if (event.button < 4) // 1-3 MBtns, 4-5 MWheel
                this->MouseDown(event.button - 1, event.x, event.y); //can be event.x_root,y_root
            else if (event.button == 4)
                this->OnScroll(1); // btn==4 is UP,
            else
                this->OnScroll(-1); // btn==5 is DOWN
            break;
        }
        case ButtonRelease:
        {
            this->MouseUp(event.button - 1);
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
            XFree(confEvent);
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
    return std::static_cast<void*>(mWindow);
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

static int Window::ErrorHandler(::Display *dpy, XErrorEvent *error)
{
    char errorCode[1024];
    XGetErrorText(dpy, error->error_code, errorCode, 1024);

    LOG_ERROR("_X Error of failed request: %s\n"
            "_  Major opcode of failed request: %3d\n"
            "_  Serial number of failed request: %5d\n",
            errorCode, error->request_code, error->serial);
    mWindowError = true;
    return 0;
}

} // namespace Common
} // namespace NFE
