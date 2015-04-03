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

    HWND mHWND; //< Window Handle, used to init and destroy Backbuffer
    HDC mHDC;   //< Device Context for OpenGL
    HGLRC mHRC; //< Rendering Context for OpenGL

public:
    Backbuffer();
    ~Backbuffer();

    bool Resize(int newWidth, int newHeight);
    bool Init(const BackbufferDesc& desc);
    bool Present();
};

} // namespace Renderer
} // namespace NFE
