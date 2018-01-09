/**
 * @file
 * @author mkkulagowski (mkkulagowski@gmail.com)
 * @brief  Window class definition.
 */

#include "PCH.hpp"

#include "../Window.hpp"
#include "Logger/Logger.hpp"

#if 0
#include <GL/glx.h>
#endif


namespace NFE {
namespace Common {


namespace {

const char* TranslateErrorCodeToStr(int err)
{
    // these error codes base on XErrorDB, available on /usr/share/X11/XErrorDB
    switch(err)
    {
    case 0: return "OK";
    case 1: return "BadRequest";
    case 2: return "BadValue";
    case 3: return "BadWindow";
    case 4: return "BadPixmap";
    case 5: return "BadAtom";
    case 6: return "BadCursor";
    case 7: return "BadFont";
    case 8: return "BadMatch";
    case 9: return "BadDrawable";
    case 10: return "BadAccess";
    case 11: return "BadAlloc";
    case 12: return "BadColor";
    case 13: return "BadGC";
    case 14: return "BadIDChoice";
    case 15: return "BadName";
    case 16: return "BadLength";
    case 17: return "BadImplementation";
    default: return "Unknown";
    }
}

} // namespace



Window::Window()
    : mConnection(nullptr)
    , mWindow(0)
    , mScreen(nullptr)
    , mDeleteReply(nullptr)
    , mConnScreen(0)
    , mClosed(true)
    , mFullscreen(false)
    , mInvisible(false)
    , mWidth(400)
    , mHeight(400)
    , mLeft(20)
    , mTop(20)
    , mTitle("Window")
    , mResizeCallback(nullptr)
    , mResizeCallbackUserData(nullptr)
{
    for (int i = 0; i < 3; i++)
        mMouseButtons[i] = false;

    for (int i = 0; i < 255; i++)
        mKeys[i] = false;
}

Window::~Window()
{
    Close();
    free(mDeleteReply);

    if (mConnection)
    {
        xcb_set_screen_saver(mConnection, -1, 0, XCB_BLANKING_NOT_PREFERRED, XCB_EXPOSURES_ALLOWED);
        xcb_destroy_window(mConnection, mWindow);
        xcb_flush(mConnection);
        xcb_disconnect(mConnection);
    }
}

bool Window::Init()
{
    mConnection = xcb_connect(nullptr, &mConnScreen);
    if (xcb_connection_has_error(mConnection))
    {
        NFE_LOG_ERROR("Failed to connect to X server!");
        return false;
    }

    // acquire current screen
    const xcb_setup_t* xcbSetup;
    xcb_screen_iterator_t xcbIt;

    xcbSetup = xcb_get_setup(mConnection);
    xcbIt = xcb_setup_roots_iterator(xcbSetup);
    while (mConnScreen-- > 0)
        xcb_screen_next(&xcbIt);
    mScreen = xcbIt.data;

    xcb_set_screen_saver(mConnection, 0, 0, XCB_BLANKING_NOT_PREFERRED, XCB_EXPOSURES_ALLOWED);
    return true;
}

void Window::SetSize(uint32 width, uint32 height)
{
    mWidth = width;
    mHeight = height;

    if (!mClosed)
    {
        const uint32 newSize[] = { mWidth, mHeight };
        xcb_configure_window(mConnection, mWindow,
                             XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, newSize);
    }
}

void Window::SetTitle(const char* title)
{
    mTitle = title;
    if (!mClosed)
    {
        xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, mWindow, XCB_ATOM_WM_NAME,
                            XCB_ATOM_STRING, 8, mTitle.size(), mTitle.c_str());
    }
}

void Window::SetFullscreenMode(bool enabled)
{
    if (!mClosed)
    {
        // TODO here is only done hinting X11 that the screen will be fullscreen.
        //      What needs to be added is switching video mode using XF86, RandR or sth else.

        // acquire cookies
        xcb_intern_atom_cookie_t wmStateCookie = xcb_intern_atom(mConnection, 1, 13, "_NET_WM_STATE");
        xcb_intern_atom_reply_t* wmState = xcb_intern_atom_reply(mConnection, wmStateCookie, 0);
        xcb_intern_atom_cookie_t fsCookie = xcb_intern_atom(mConnection, 1, 24, "_NET_WM_STATE_FULLSCREEN");
        xcb_intern_atom_reply_t* fs = xcb_intern_atom_reply(mConnection, fsCookie, 0);

        if (mFullscreen && !enabled)
        {
            xcb_client_message_event_t event;
            memset(&event, 0, sizeof(event));
            event.response_type = XCB_CLIENT_MESSAGE;
            event.window = mScreen->root;
            event.type = wmState->atom;
            event.format = 32;
            event.data.data32[0] = 0; // _NET_WM_STATE_REMOVE
            event.data.data32[1] = fs->atom;
            event.data.data32[2] = 0;

            xcb_send_event(mConnection, 0, mScreen->root,
                           XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                           reinterpret_cast<const char*>(&event));
            xcb_flush(mConnection);
        }
        else if (!mFullscreen && enabled)
        {
            xcb_client_message_event_t event;
            memset(&event, 0, sizeof(event));
            event.response_type = XCB_CLIENT_MESSAGE;
            event.window = mScreen->root;
            event.type = wmState->atom;
            event.format = 32;
            event.data.data32[0] = 1; //_NET_WM_STATE_ADD
            event.data.data32[1] = fs->atom;
            event.data.data32[2] = 0;

            xcb_send_event(mConnection, 0, mScreen->root,
                           XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                           reinterpret_cast<const char*>(&event));
            xcb_flush(mConnection);
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
            xcb_unmap_window(mConnection, mWindow);
        else
            xcb_map_window(mConnection, mWindow);
    }
}

bool Window::Open()
{
    if (!mClosed)
        return false;

    // TODO disabled until OpenGL renderer will resurrect from the dead
    // If this code is to be revived, some XCB-Xlib
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
    NFE_LOG_INFO("Found %d matching FB configs:", fbCount);
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
            NFE_LOG_INFO("  #%d: visualID 0x%2lu, SAMPLE_BUFFERS = %d, SAMPLES = %d",
                     i, vi->visualid, sampleBuffers, samples);

            if (samples < maxSamples)
            {
                bestFBID = i;
                maxSamples = samples;
            }
        }
        XFree(vi);
    }

    NFE_LOG_INFO("Choosing FB config #%d", bestFBID);
    GLXFBConfig bestFB = fbc[bestFBID];
    XFree(fbc);

    XVisualInfo* visual = glXGetVisualFromFBConfig(mDisplay, bestFB);
#endif

    // generate our window ID
    mWindow = xcb_generate_id(mConnection);

    xcb_colormap_t colormap = xcb_generate_id(mConnection);
    xcb_create_colormap(mConnection, XCB_COLORMAP_ALLOC_NONE, colormap, mScreen->root,
                        mScreen->root_visual);

    // set some settings (these match the old ones)
    uint32 value_mask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;
    uint32 value_list[] = {
        XCB_EVENT_MASK_BUTTON_1_MOTION | XCB_EVENT_MASK_BUTTON_2_MOTION | XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_FOCUS_CHANGE |
        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY,
        colormap
    };

    xcb_void_cookie_t cookie = xcb_create_window_checked(mConnection, mScreen->root_depth, mWindow,
                                                      mScreen->root, mLeft, mTop, mWidth, mHeight,
                                                      1, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                                      mScreen->root_visual, value_mask, value_list);

    xcb_generic_error_t* err = xcb_request_check(mConnection, cookie);
    if (err)
    {
        NFE_LOG_ERROR("Failed to create a window: X11 protocol error %d (%s)", err->error_code,
                  TranslateErrorCodeToStr(err->error_code));
        free(err);
        return false;
    }

    xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, mWindow, XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING, 8, mTitle.size(), mTitle.c_str());

    // pre-initialize delete atom to be later on notified about Window being destroyed
    xcb_intern_atom_cookie_t deleteCookie = xcb_intern_atom(mConnection, 1, 16, "WM_DELETE_WINDOW");
    mDeleteReply = xcb_intern_atom_reply(mConnection, deleteCookie, nullptr);

    // get access to WM_PROTOCOLS atom
    xcb_intern_atom_cookie_t wmProtCookie = xcb_intern_atom(mConnection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* wmProtReply = xcb_intern_atom_reply(mConnection, wmProtCookie, nullptr);

    // notify X that we want to be part of WM_DELETE_WINDOW communication
    xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, mWindow, wmProtReply->atom, 4, 32, 1,
                        &mDeleteReply->atom);
    free(wmProtReply);

    if (!mInvisible)
        xcb_map_window(mConnection, mWindow);

    mClosed = false;
    return true;
}

bool Window::Close()
{
    if (mClosed)
        return false;

    if (!mInvisible)
        xcb_unmap_window(mConnection, mWindow);

    mClosed = true;
    return true;
}

void Window::MouseDown(uint32 button, int x, int y)
{
    xcb_grab_pointer_cookie_t grab = xcb_grab_pointer(mConnection, 1, mWindow, 0,
                                                      XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                                                      mWindow, XCB_NONE, XCB_CURRENT_TIME);
    xcb_grab_pointer_reply_t* grabReply = xcb_grab_pointer_reply(mConnection, grab, nullptr);
    free(grabReply);

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
        xcb_ungrab_pointer(mConnection, XCB_CURRENT_TIME);

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
    xcb_flush(mConnection);

    xcb_generic_event_t* event;
    while ((event = xcb_poll_for_event(mConnection)))
    {
        // I have no idea why (and XCB reference won't tell me why as well),
        // but without & ~0x80 ClientMessage event is not received
        switch (event->response_type & ~0x80)
        {

            case XCB_CLIENT_MESSAGE:
            {
                xcb_client_message_event_t* cm = reinterpret_cast<xcb_client_message_event_t*>(event);

                // close handling
                if (static_cast<xcb_atom_t>(cm->data.data32[0]) == mDeleteReply->atom)
                {
                    this->OnClose();
                    this->Close();
                }
                break;
            }
            case XCB_KEY_PRESS:
            {
                xcb_key_press_event_t* kp = reinterpret_cast<xcb_key_press_event_t*>(event);
                mKeys[kp->detail] = true;
                OnKeyPress(static_cast<KeyCode>(kp->detail));
                break;
            }
            case XCB_KEY_RELEASE:
            {
                xcb_key_release_event_t* kr = reinterpret_cast<xcb_key_release_event_t*>(event);
                this->mKeys[kr->detail] = false;
                OnKeyUp(static_cast<KeyCode>(kr->detail));
                break;
            }
            case XCB_MOTION_NOTIFY:
            {
                xcb_motion_notify_event_t* m = reinterpret_cast<xcb_motion_notify_event_t*>(event);
                this->MouseMove(m->event_x, m->event_y);
                break;
            }
            case XCB_BUTTON_PRESS:
            {
                xcb_button_press_event_t* bp = reinterpret_cast<xcb_button_press_event_t*>(event);

                if (bp->detail < 4) // 1-3 MBtns, 4-5 MWheel
                    this->MouseDown(bp->detail - 1, bp->event_x, bp->event_y);
                else if (bp->detail == 4)
                    this->OnScroll(1); // btn==4 is UP,
                else
                    this->OnScroll(-1); // btn==5 is DOWN
                break;
            }
            case XCB_BUTTON_RELEASE:
            {
                xcb_button_release_event_t* br = reinterpret_cast<xcb_button_release_event_t*>(event);
                this->MouseUp(br->detail - 1);
                break;
            }
            case XCB_FOCUS_OUT:
            {
                this->LostFocus();
                break;
            }
            case XCB_CONFIGURE_NOTIFY:
            {
                xcb_configure_notify_event_t* cn = reinterpret_cast<xcb_configure_notify_event_t*>(event);

                if (static_cast<uint32>(cn->width) != mWidth ||
                    static_cast<uint32>(cn->height) != mHeight)
                {
                    mWidth = cn->width;
                    mHeight = cn->height;
                    this->OnResize(mWidth, mHeight);

                    if (mResizeCallback)
                        mResizeCallback(mResizeCallbackUserData);
                }
                break;
            }
        }

        free(event);
    }
}

void Window::LostFocus()
{
    MouseUp(0);
    MouseUp(1);
    MouseUp(2);

    for (int i = 0; i < NFE_WINDOW_KEYS_NUM; i++)
    {
        if (mKeys[i])
        {
            OnKeyUp(static_cast<KeyCode>(i));
            mKeys[i] = false;
        }
    }
}

bool Window::IsClosed() const
{
    return mClosed;
}

bool Window::HasFocus() const
{
    xcb_get_input_focus_cookie_t inputCookie = xcb_get_input_focus(mConnection);
    xcb_get_input_focus_reply_t* input = xcb_get_input_focus_reply(mConnection, inputCookie, nullptr);
    return input->focus == mWindow;
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
    NFE_UNUSED(width);
    NFE_UNUSED(height);
}

void Window::OnKeyPress(KeyCode key)
{
    NFE_UNUSED(key);
}

void Window::OnKeyUp(KeyCode key)
{
    NFE_UNUSED(key);
}

void Window::OnCharTyped(const char* charUTF8)
{
    NFE_UNUSED(charUTF8);
}

void Window::OnScroll(int delta)
{
    NFE_UNUSED(delta);
}

void Window::OnMouseDown(uint32 button, int x, int y)
{
    NFE_UNUSED(button);
    NFE_UNUSED(x);
    NFE_UNUSED(y);
}

void Window::OnMouseMove(int x, int y, int deltaX, int deltaY)
{
    NFE_UNUSED(x);
    NFE_UNUSED(y);
    NFE_UNUSED(deltaX);
    NFE_UNUSED(deltaY);
}

void Window::OnMouseUp(uint32 button)
{
    NFE_UNUSED(button);
}

} // namespace Common
} // namespace NFE
