/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  OpenGL Master Context singleton declaration
 */

#pragma once

#include "Defines.hpp"


namespace NFE {
namespace Renderer {


class MasterContext
{
    MasterContext();
    MasterContext(const MasterContext&) = delete;
    MasterContext(MasterContext&&) = delete;
    MasterContext& operator=(const MasterContext&) = delete;
    MasterContext& operator=(MasterContext&&) = delete;
    ~MasterContext();

    void* mData; ///< Context-dependent data. Each platform sets its own packet here.

#if defined(WIN32)
    // TODO fill
#elif defined(__linux__) || defined(__LINUX__)
    Pixmap mPixmap;
    GLXPixmap mGLXPixmap; ///< Pixmap used for off-window rendering context
    GLXDrawable mDrawable;
#endif

public:
    /**
     * Acquire an instance of Context object.
     *
     * @return Reference to Context object instance.
     */
    static MasterContext& Instance();

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
