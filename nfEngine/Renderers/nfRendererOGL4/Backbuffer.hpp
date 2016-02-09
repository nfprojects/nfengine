/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's backbuffer
 */

#include "../RendererInterface/Backbuffer.hpp"
#include "Texture.hpp"

namespace NFE {
namespace Renderer {

// TODO: Linux implementation of Backbuffer methods
class Backbuffer : public IBackbuffer, public Texture
{
    friend class CommandBuffer;

#ifdef WIN32
    HWND mHWND; //< Window Handle, used to init and destroy Backbuffer
    HDC mHDC;   //< Device Context for OpenGL
    HGLRC mHRC; //< Rendering Context for OpenGL
#elif defined(__linux__) | defined(__LINUX__)
    Window mWindow; //< Xorg Window pointer, extracted from NFE::Common::Window
    Display* mMasterDisplay;
    GLXContext mContext;
    GLXDrawable mDrawable;
    GLuint mDummyVAO;
#else
#error "Target platform not supported."
#endif

public:
    Backbuffer();
    ~Backbuffer();

    bool Resize(int newWidth, int newHeight);
    bool Init(const BackbufferDesc& desc);
    bool Present();
};

} // namespace Renderer
} // namespace NFE
