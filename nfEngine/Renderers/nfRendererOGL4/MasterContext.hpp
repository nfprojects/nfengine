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

    // unique_ptr is unusable here due to void being an incomplete type
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
     * After a successful call to this function, it is assured that MasterContext helds a valid
     * context-dependent data packet, which can be used to create Slave Contexts.
     */
    bool Init();

    /**
     * Release a Master Context.
     *
     * Calling this function will remove the Master Context. All additional cleanup work is done
     * (ex. closing a connection to X server), so Slave Contexts might be invalid after this call.
     */
    void Release();

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
