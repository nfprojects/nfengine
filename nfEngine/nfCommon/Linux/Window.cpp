/**
 * @file
 * @author mkkulagowski (mkkulagowski@gmail.com)
 * @brief  Window class definition.
 */

#include "PCH.hpp"

#include "../Window.hpp"
#include "../Logger.hpp"

#if 0
#include <GL/glx.h>
#endif

#include <X11/Xutil.h>

namespace NFE {
namespace Common {

bool Window::mWindowError;

Window::Window()
{
    mWindow = 0;
    mClosed = true;
    mWidth = 200;
    mHeight = 200;
    mFullscreen = false;
    mInvisible = false;
    mResizeCallback = nullptr;
    mResizeCallbackUserData = nullptr;
    mTitle = "Window";

    mDisplay = XOpenDisplay(nullptr);
    if (mDisplay == nullptr)
        LOG_ERROR("Failed to connect to X server\n");

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

void Window::SetInvisible(bool invisible)
{
    mInvisible = invisible;

    if (mWindow)
    {
        if (mInvisible)
            XUnmapWindow(mDisplay, mWindow);
        else
            XMapWindow(mDisplay, mWindow);
    }
}

bool Window::Open()
{
    if (!mClosed)
        return false;

    ::XSetWindowAttributes xSetWAttrib;

    // TODO disabled until OpenGL renderer will resurrect from the dead
#if 0
    // Visual should be chosen with proper FB Config
    static int fbAttribs[] =
    {
        GLX_X_RENDERABLE,  True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE,      8,
        GLX_GREEN_SIZE,    8,
        GLX_BLUE_SIZE,     8,
        GLX_ALPHA_SIZE,    8,
        GLX_DEPTH_SIZE,    24,
        GLX_STENCIL_SIZE,  8,
        GLX_DOUBLEBUFFER,  True,
        None
    };

    int fbCount;
    GLXFBConfig* fbc = glXChooseFBConfig(mDisplay, DefaultScreen(mDisplay), fbAttribs, &fbCount);

    // TODO options should allow to select Multisampling level during init
    LOG_INFO("Found %d matching FB configs:", fbCount);
    // Select the best FB Config according to highest GLX_SAMPLES attribute value
    int bestFBID = -1, maxSamples = 16;
    for (int i = 0; i < fbCount; ++i)
    {
        XVisualInfo* vi = glXGetVisualFromFBConfig(mDisplay, fbc[i]);
        if (vi)
        {
            int sampleBuffers;
            int samples;
            glXGetFBConfigAttrib(mDisplay, fbc[i], GLX_SAMPLE_BUFFERS, &sampleBuffers);
            glXGetFBConfigAttrib(mDisplay, fbc[i], GLX_SAMPLES, &samples);
            LOG_INFO("  #%d: visualID 0x%2lu, SAMPLE_BUFFERS = %d, SAMPLES = %d",
                     i, vi->visualid, sampleBuffers, samples);

            if (samples < maxSamples)
            {
                bestFBID = i;
                maxSamples = samples;
            }
        }
        XFree(vi);
    }

    LOG_INFO("Choosing FB config #%d", bestFBID);
    GLXFBConfig bestFB = fbc[bestFBID];
    XFree(fbc);

    XVisualInfo* visual = glXGetVisualFromFBConfig(mDisplay, bestFB);
#endif

    ::Visual* visual = XDefaultVisual(mDisplay, 0);
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

    if (!mInvisible)
        XMapWindow(mDisplay, mWindow);

    mClosed = false;
    return true;
}

bool Window::Close()
{
    if (mClosed)
        return false;

    if (!mInvisible)
        XUnmapWindow(mDisplay, mWindow);

    mClosed = true;
    return true;
}

void Window::MouseDown(uint32 button, int x, int y)
{
    XGrabPointer(mDisplay, mWindow, True, 0, GrabModeAsync,
                GrabModeAsync, mWindow, None, CurrentTime);
    mMouseButtons[button] = true;
    mMousePos[button] = x;
    mMousePos[button] = y;

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
    OnMouseMove(x, y, x - mMousePos[0], y - mMousePos[0]);
    mMousePos[0] = x;
    mMousePos[0] = y;
}

void Window::ProcessMessages()
{
    XFlush(mDisplay);
    while (XEventsQueued(mDisplay, QueuedAlready))
    {
        XEvent event;
        XNextEvent(mDisplay, &event);
        switch (event.type)
        {
            case ClientMessage:
            {
                if (static_cast<Atom>(event.xclient.data.l[0]) ==
                    XInternAtom(mDisplay, "WM_DELETE_WINDOW", false))
                {
                    this->Close();
                    this->OnClose();
                }
                break;
            }
            case KeyPress:
            {
                mKeys[event.xkey.keycode] = true;
                OnKeyPress(static_cast<KeyCode>(event.xkey.keycode));
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
                    //can be event.x_root,y_root
                    this->MouseDown(event.xbutton.button - 1, event.xbutton.x, event.xbutton.y);
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

                if (static_cast<uint32>(confEvent.width) != mWidth ||
                    static_cast<uint32>(confEvent.height) != mHeight)
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
    int revert;
    ::Window window;
    XGetInputFocus(mDisplay, &window, &revert);
    return mWindow == window;
}

void* Window::GetHandle() const
{
    return reinterpret_cast<void*>(mWindow);
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

int Window::ErrorHandler(::Display *dpy, XErrorEvent *error)
{
    char errorCode[1024];
    XGetErrorText(dpy, error->error_code, errorCode, 1024);
    LOG_ERROR("_X Error of failed request: %s\n", errorCode);
    Window::mWindowError = true;
    return 0;
}

} // namespace Common
} // namespace NFE
