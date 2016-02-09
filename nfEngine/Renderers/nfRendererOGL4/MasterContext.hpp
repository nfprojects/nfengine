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
    friend class Backbuffer;

    MasterContext();
    MasterContext(const MasterContext&) = delete;
    MasterContext(MasterContext&&) = delete;
    MasterContext& operator=(const MasterContext&) = delete;
    MasterContext& operator=(MasterContext&&) = delete;
    ~MasterContext();

#if defined(WIN32)
    // TODO fill
#elif defined(__linux__) || defined(__LINUX__)
    Pixmap mPixmap;
    GLXPixmap mGLXPixmap; ///< Pixmap used for off-window rendering context
    GLXDrawable mDrawable;
    Display* mDisplay; // TODO replace with std::shared_ptr
    GLXFBConfig mBestFB;
    GLXContext mContext;
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
};


} // namespace Renderer
} // namespace NFE
