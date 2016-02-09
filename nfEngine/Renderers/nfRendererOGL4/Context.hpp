/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OpenGL Main Context singleton declaration
 */

#pragma once

#include "Defines.hpp"


namespace NFE {
namespace Renderer {


class Context
{
    Context();
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
    ~Context();

    void* mData; ///< Context-dependent data. Each platform sets its own packet here.

#if defined(WIN32)
    // TODO fill
#elif defined(__linux__) || defined(__LINUX__)
    Display* mDisplay;
    Pixmap mPixmap;
    GLXPixmap mGLXPixmap; ///< Pixmap used for off-window rendering context
    GLXContext mContext;
    GLXDrawable mDrawable;
#endif

public:
    /**
     * Acquire an instance of Context object.
     *
     * @return Reference to Context object instance.
     */
    static Context& Instance();

    /**
     * Performs an initialization of Context instance.
     *
     * @return True if succeeded, false otherwise.
     *
     * After a call to this function, it is assured that
     */
    bool Init();

    /**
     * Acquire a pointer to Context's personal data.
     *
     * @return Pointer to Context's data
     *
     * The contents of this pointer vary depending on used platform. The data should not be
     * acquired by platform-independent code.
     */
    const void* GetData();
};


} // namespace Renderer
} // namespace NFE
