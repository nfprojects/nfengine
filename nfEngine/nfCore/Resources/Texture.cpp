/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Texture resource definition.
 */

#include "PCH.hpp"
#include "Globals.hpp"
#include "Engine.hpp"
#include "Texture.hpp"
#include "Renderer/HighLevelRenderer.hpp"
#include "Renderer/GuiRenderer.hpp"
#include "ResourcesManager.hpp"

#include "nfCommon/Logger.hpp"
#include "nfCommon/InputStream.hpp"
#include "nfCommon/Image/Image.hpp"

namespace NFE {
namespace Resource {

namespace {

const size_t gMaxMipmaps = 24;

Renderer::ITexture* CreateRendererTextureFromImage(const Common::Image& image,
                                                   const char* debugName = nullptr)
{
    using namespace Renderer;

    size_t bitsPerPixel = Common::BitsPerPixel(image.GetFormat());

    TextureDesc texDesc;
    texDesc.type = TextureType::Texture2D;
    texDesc.access = BufferAccess::GPU_ReadOnly;
    texDesc.width = image.GetWidth();
    texDesc.height = image.GetHeight();
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER;
    texDesc.mipmaps = static_cast<int>(image.GetMipmapsNum());
    texDesc.debugName = debugName;

    bool bc = false;
    size_t bcNumBytesPerBlock = 0;
    switch (image.GetFormat())
    {
    case Common::ImageFormat::A_UByte:
    case Common::ImageFormat::R_UByte:
        texDesc.format = ElementFormat::Uint_8_norm;
        texDesc.texelSize = 1;
        break;
    case Common::ImageFormat::RGBA_UByte:
        texDesc.format = ElementFormat::Uint_8_norm;
        texDesc.texelSize = 4;
        break;
    case Common::ImageFormat::RGBA_Float:
        texDesc.format = ElementFormat::Float_32;
        texDesc.texelSize = 4;
        break;
    case Common::ImageFormat::R_Float:
        texDesc.format = ElementFormat::Float_32;
        texDesc.texelSize = 1;
        break;
    case Common::ImageFormat::BC1:
        texDesc.format = ElementFormat::BC1;
        texDesc.texelSize = 1;
        bcNumBytesPerBlock = 8;
        bc = true;
        break;
    case Common::ImageFormat::BC2:
        texDesc.format = ElementFormat::BC2;
        texDesc.texelSize = 1;
        bcNumBytesPerBlock = 16;
        bc = true;
        break;
    case Common::ImageFormat::BC3:
        texDesc.format = ElementFormat::BC3;
        texDesc.texelSize = 1;
        bcNumBytesPerBlock = 16;
        bc = true;
        break;
    case Common::ImageFormat::BC4:
        texDesc.format = ElementFormat::BC4;
        texDesc.texelSize = 1;
        bcNumBytesPerBlock = 8;
        bc = true;
        break;
    case Common::ImageFormat::BC5:
        texDesc.format = ElementFormat::BC5;
        texDesc.texelSize = 1;
        bcNumBytesPerBlock = 16;
        bc = true;
        break;
    default:
        return nullptr;
    }

    TextureDataDesc initialData[gMaxMipmaps];
    uint32 mipmapNum = static_cast<uint32>(image.GetMipmapsNum());
    for (uint32 i = 0; i < mipmapNum; i++)
    {
        if (bc) //special case - block coding
        {
            uint32 numBlocksWide = Math::Max<uint32>(1, (image.GetMipmap(i)->GetWidth() + 3) / 4);
            initialData[i].lineSize = numBlocksWide * bcNumBytesPerBlock;
        }
        else
        {
            uint32 lineSize = image.GetMipmap(i)->GetWidth() * static_cast<uint32>(bitsPerPixel) / 8;
            initialData[i].lineSize = Math::Max<uint32>(1, lineSize);
        }

        initialData[i].data = image.GetData(i);
        initialData[i].sliceSize = image.GetMipmap(i)->GetDataSize();
    }

    texDesc.dataDesc = initialData;

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    return renderer->GetDevice()->CreateTexture(texDesc);
}

} // namespace

Texture::Texture()
{
    mTex = nullptr;
    mFormat = Common::ImageFormat::Unknown;
}

Texture::~Texture()
{
    Release();
}

void Texture::Release()
{
    std::recursive_mutex& renderingMutex = Engine::GetInstance()->GetRenderingMutex();
    std::unique_lock<std::recursive_mutex> lock(renderingMutex);

    mTex.reset();
}

bool Texture::OnLoad()
{
    LOG_INFO("Loading texture '%s'...", mName);
    Common::Timer timer;
    timer.Start();

    if (mOnLoad)
    {
        if (!mOnLoad(this, mUserPtr))
        {
            LOG_ERROR("Failed to load '%s'.", mName);
            return false;
        }
    }
    else
    {
        //get relative path
        std::string path = g_DataPath + "Textures/" + mName;

        Common::File file;
        if (!file.Open(path, Common::AccessMode::Read))
        {
            LOG_ERROR("Could not open texture '%s'!", path.c_str());
            return false;
        }

        Common::Image image;
        Common::FileInputStream stream(file);

        if (!image.Load(&stream))
        {
            LOG_ERROR("Failed to open '%s'.", mName);
            return false;
        }
        LOG_SUCCESS("File '%s' loaded.", mName);


        // generate mipmaps only for non-block-coded formats
        mFormat = image.GetFormat();
        if (mFormat != Common::ImageFormat::BC1 && mFormat != Common::ImageFormat::BC2 &&
                mFormat != Common::ImageFormat::BC3 && mFormat != Common::ImageFormat::BC4 &&
                mFormat != Common::ImageFormat::BC5 && mFormat != Common::ImageFormat::Unknown)
        {
            image.GenerateMipmaps(Common::MipmapFilter::Box);
        }

        // RGBA_UByte is not supported by D3D
        if (mFormat == Common::ImageFormat::RGB_UByte)
        {
            mFormat = Common::ImageFormat::RGBA_UByte;
            image.Convert(mFormat);
        }


        // TODO: pack file support
        /*
        Buffer buffer;
        Common::Image image;
        if (g_pVfsReader->Read(path, buffer) != PACK_OK)
        {
            //oepn from file
            strcpy(path, "..\\Data\\Textures\\");
            strcat(path, mName);
            if (image.LoadFromFile(path) != true)
            {
                LOG_ERROR("Failed to open '%s'.", mName);
                return false;
            }
            LOG_SUCCESS("File '%s' loaded.", mName);
        }
        else
        {
            if (image.LoadFromBuffer(buffer.GetData(), buffer.GetSize()) != true)
            {
                LOG_ERROR("Failed to open '%s'.", mName);
                return false;
            }
            LOG_SUCCESS("File '%s' loaded from PAK file.", mName);
        }
        */

        if (CreateFromImage(image) != true)
            return false;

        LOG_SUCCESS("Texture '%s' loaded in %.3f sec. Dim: %ix%i, format: %s.",
                    mName, timer.Stop(), image.GetWidth(), image.GetHeight(),
                    Common::FormatToStr(image.GetFormat()));
    }
    return true;
}

void Texture::OnUnload()
{
    if (mOnUnload)
        mOnUnload(this, mUserPtr);

    LOG_INFO("Unloading texture '%s'...", mName);
    Release();
}



// Load the texture from a stream (supported formats: BMP, JPEG and PNG).
bool Texture::CreateFromStream(Common::InputStream* pStream)
{
    return true;
}

// Create the texture from a custom memory buffer.
bool Texture::CreateFromImage(const Common::Image& image)
{
    Release();

    {
        std::recursive_mutex& renderingMutex = Engine::GetInstance()->GetRenderingMutex();
        std::unique_lock<std::recursive_mutex> lock(renderingMutex);

        mTex.reset(CreateRendererTextureFromImage(image, mName));
    }

    if (!mTex)
    {
        LOG_ERROR("Failed to create renderer's texture for '%s'.", mName);
        return false;
    }

    return true;
}


using namespace Renderer;

bool Texture::CreateAsRenderTarget(uint32 width, uint32 height, Renderer::ElementFormat format)
{
    Release();

    TextureDesc texDesc;
    texDesc.type = TextureType::Texture2D;
    texDesc.access = BufferAccess::GPU_ReadWrite;
    texDesc.width = width;
    texDesc.height = height;
    texDesc.binding = NFE_RENDERER_TEXTURE_BIND_SHADER | NFE_RENDERER_TEXTURE_BIND_RENDERTARGET;
    texDesc.mipmaps = 1;
    texDesc.debugName = "Texture::mTex (render target)";
    texDesc.format = format;
    texDesc.texelSize = 4;

    HighLevelRenderer* renderer = Engine::GetInstance()->GetRenderer();
    mTex.reset(renderer->GetDevice()->CreateTexture(texDesc));
    if (!mTex)
    {
        LOG_ERROR("Failed to create render target texture");
        return false;
    }

    mTexBinding = GuiRenderer::Get()->CreateTextureBinding(mTex.get());
    if (!mTexBinding)
        return false;

    return true;
}

Renderer::ITexture* Texture::GetRendererTexture() const
{
    return mTex.get();
}

IResourceBindingInstance* Texture::GetRendererTextureBinding() const
{
    return mTexBinding.get();
}

} // namespace Resource
} // namespace NFE
