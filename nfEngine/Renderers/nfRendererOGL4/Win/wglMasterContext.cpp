/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OpenGL Master Context singleton definitions for Windows
 */

#include "../PCH.hpp"

#include "../MasterContext.hpp"
#include "Window.hpp"

namespace {
std::unique_ptr<NFE::Common::Window> gInvisibleWindow; // Invisible window for off-screen rendering
} // namespace

namespace NFE {
namespace Renderer {


MasterContext::MasterContext()
    : mHWND(0)
    , mHDC(0)
    , mHRC(0)
{
    for (auto& attrib : mAttribs)
        attrib = 0;

    for (auto& attrib : mPixelFormatIntAttribs)
        attrib = 0;

    for (auto& attrib : mPixelFormatFloatAttribs)
        attrib = 0.0f;
}

MasterContext::~MasterContext()
{
    Release();
}

MasterContext& MasterContext::Instance()
{
    static MasterContext instance;
    return instance;
}

bool MasterContext::Init()
{
    // Create an invisible window for our Master Context
    gInvisibleWindow.reset(new NFE::Common::Window());
    gInvisibleWindow->SetInvisible(true);
    gInvisibleWindow->Open();

    // Create a dummy context to extract WGL_ARB_context_attribs
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARBFunc;
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARBFunc;
    {
        PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1,
                                      PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
                                      PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24,
                                      8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0 };


        mHDC = GetDC(static_cast<HWND>(gInvisibleWindow->GetHandle()));
        if (!mHDC)
        {
            NFE_LOG_ERROR("Failed to get invisible window DC.");
            return false;
        }

        unsigned int pixelFormat = ChoosePixelFormat(mHDC, &pfd);
        if (!pixelFormat)
        {
            NFE_LOG_ERROR("Unable to choose a Pixel Format for dummy context.");
            return false;
        }

        if (!SetPixelFormat(mHDC, pixelFormat, &pfd))
        {
            NFE_LOG_ERROR("Unable to set a Pixel Format for dummy context.");
            return false;
        }

        // The old-way OpenGL context creation
        mHRC = wglCreateContext(mHDC);
        if (!mHRC)
        {
            NFE_LOG_ERROR("Failed to create dummy OpenGL context.");
            return false;
        }

        // Make our context current
        wglMakeCurrent(mHDC, mHRC);

        // Extract whatever we need
        // We must do it outside of Extensions.hpp, because this extraction is not bound with
        // our target context. Otherwise, we would have to re-acquire the extensions to check
        // OpenGL version compatibility.
        wglCreateContextAttribsARBFunc =
            (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        wglChoosePixelFormatARBFunc =
            (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        if (!wglCreateContextAttribsARBFunc || !wglChoosePixelFormatARBFunc)
        {
            NFE_LOG_ERROR("Failed to retrieve WGL context creators.");
            return false;
        }

        // Function extracted, no need for our dummy context now
        wglMakeCurrent(0, 0);
        wglDeleteContext(mHRC);
        mHRC = 0;
    }

    int i = 0;
    mPixelFormatIntAttribs[i++] = WGL_SUPPORT_OPENGL_ARB;
    mPixelFormatIntAttribs[i++] = GL_TRUE;
    mPixelFormatIntAttribs[i++] = WGL_COLOR_BITS_ARB;
    mPixelFormatIntAttribs[i++] = 32;
    mPixelFormatIntAttribs[i++] = WGL_DEPTH_BITS_ARB;
    mPixelFormatIntAttribs[i++] = 24;
    mPixelFormatIntAttribs[i++] = WGL_STENCIL_BITS_ARB;
    mPixelFormatIntAttribs[i++] = 8;
    mPixelFormatIntAttribs[i++] = WGL_DOUBLE_BUFFER_ARB;
    mPixelFormatIntAttribs[i++] = GL_TRUE;
    mPixelFormatIntAttribs[i++] = 0;

    mPixelFormatFloatAttribs[0] = 0.0f;

    int pixelFormat = 0;
    unsigned int numFormats = 0;
    if (!wglChoosePixelFormatARBFunc(mHDC, mPixelFormatIntAttribs, mPixelFormatFloatAttribs, 1,
                                     &pixelFormat, &numFormats))
    {
        NFE_LOG_ERROR("Failed to choose pixel format.");
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    if (!SetPixelFormat(mHDC, pixelFormat, &pfd))
    {
        NFE_LOG_ERROR("Failed to set chosen pixel format.");
        return false;
    }

    // TODO whatever happens in this struct can now adjust our context settings.
    //      Consider making below fields configurable, both in Linux and Windows.
    // TODO debug context
    int attribCount = 0;

    // use Core Profile
    mAttribs[attribCount++] = WGL_CONTEXT_PROFILE_MASK_ARB;
    mAttribs[attribCount++] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;

    // disable deprecated APIs
    mAttribs[attribCount++] = WGL_CONTEXT_FLAGS_ARB;
    mAttribs[attribCount++] = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

    // select appropriate context version
    mAttribs[attribCount++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
    mAttribs[attribCount++] = 3;
    mAttribs[attribCount++] = WGL_CONTEXT_MINOR_VERSION_ARB;
    mAttribs[attribCount++] = 3;

    mAttribs[attribCount++] = 0;

    mHRC = wglCreateContextAttribsARBFunc(mHDC, 0, mAttribs);
    if (!mHRC)
    {
        NFE_LOG_ERROR("Failed to create Core Context.");
        return false;
    }

    if (!wglMakeCurrent(mHDC, mHRC))
    {
        NFE_LOG_ERROR("Failed to make Core Context current.");
        return false;
    }

    // Acquire OGL extensions
    if (!nfglExtensionsInit())
        return false;

    // For information purposes - print what OpenGL Context we achieved
    const GLubyte* glv = glGetString(GL_VERSION);
    const char* glvStr = reinterpret_cast<const char*>(glv);
    NFE_LOG_INFO("OpenGL %s Master Context obtained.", glvStr);

    return true;
}

void MasterContext::Release()
{
    if (mHRC)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(mHRC);
        mHRC = nullptr;
    }

    if (mHDC)
    {
        ReleaseDC(mHWND, mHDC);
        mHDC = nullptr;
    }

    gInvisibleWindow.reset();
}

} // namespace Renderer
} // namespace NFE
