/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Texture resource declaration.
 */

#pragma once

#include "../Core.hpp"
#include "Resource.hpp"
#include "nfCommon/InputStream.hpp"
#include "Renderers/RendererInterface/Device.hpp"

namespace NFE {
namespace Resource {

/**
 * Texture resource.
 * @remarks This class can be inherited to create custom textures.
 */
class CORE_API Texture : public ResourceBase
{
    std::unique_ptr<Renderer::ITexture> mTex;
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
    bool CreateFromStream(Common::InputStream* pStream);

    /**
     * Create the texture from a custom memory buffer.
     */
    bool CreateFromImage(const Common::Image& image);

    /**
     * Create renderable texture, for example to show monitor screen with a view from a camera.
     * @remarks Should be used only by View class.
     * @param width,height Texture dimensions
     * @param format       Texture format
     * @return True on success
     */
    bool CreateAsRenderTarget(uint32 width, uint32 height, Renderer::ElementFormat format =
                              Renderer::ElementFormat::Uint_8_norm);

    /**
     * Get low-level renderer texture object.
     */
    Renderer::ITexture* GetRendererTexture() const;
};

} // namespace Resource
} // namespace NFE
