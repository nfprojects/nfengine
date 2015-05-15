/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Texture resource declaration.
 */

#pragma once

#include "Core.hpp"
#include "Resource.hpp"
#include "../nfCommon/InputStream.hpp"

namespace NFE {
namespace Resource {

/**
 * Texture resource.
 * @remakrs This class can be inherited to create custom textures.
 */
class CORE_API Texture : public ResourceBase
{
    Render::IRendererTexture* mTex;
    Common::ImageFormat mFormat;

    bool OnLoad();
    void OnUnload();
    void Release();

public:
    Texture();
    virtual ~Texture();

    /**
     * Load the texture from a stream (supported formats: BMP, JPEG and PNG).
     */
    Result CreateFromStream(Common::InputStream* pStream);

    /**
     * Create the texture from a custom memory buffer.
     */
    Result CreateFromImage(const Common::Image& image);

    /**
     * Create renderable texture, for example to show monitor screen with a view from a camera.
     * @remakrs Should be used only by View class.
     */
    Render::IRenderTarget* CreateRendertarget(uint32 width, uint32 height, Common::ImageFormat format);

    Render::IRendererTexture* GetRendererTexture() const;
};

} // namespace Resource
} // namespace NFE
