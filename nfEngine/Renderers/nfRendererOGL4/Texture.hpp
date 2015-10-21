/**
 * @file
 * @author  LKostyra (costyrra.xl@gmail.com)
 * @brief   Declaration of renderer's Texture object
 */

#include "../RendererInterface/Texture.hpp"

namespace NFE {
namespace Renderer {

class Texture : virtual public ITexture
{
    friend class CommandBuffer;
    friend class RenderTarget;

protected:
    TextureType type;
    int mWidth;
    int mHeight;
    int mTexelSize; //< texel size in bytes

    union
    {
        /* TODO
        ID3D11Texture1D* mTexture1D;
        ID3D11Texture2D* mTexture2D;
        ID3D11Texture3D* mTexture3D;*/
        void* mTextureGeneric;
    };

    bool InitTexture1D(const TextureDesc& desc);
    bool InitTexture2D(const TextureDesc& desc);
	bool InitTexture3D(const TextureDesc& desc);

public:
    Texture();
    virtual ~Texture();
    bool Init(const TextureDesc& desc);
};

} // namespace Renderer
} // namespace NFE
