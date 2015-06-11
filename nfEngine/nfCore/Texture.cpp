/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Texture resource definition.
 */

#include "PCH.hpp"
#include "Globals.hpp"
#include "Texture.hpp"
#include "Renderer.hpp"
#include "ResourcesManager.hpp"
#include "../nfCommon/Logger.hpp"
#include "../nfCommon/InputStream.hpp"
#include "../nfCommon/Image/Image.hpp"

namespace NFE {
namespace Resource {

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
    if (mTex)
    {
        delete mTex;
        mTex = 0;
    }
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

        Common::Image image;
        Common::InputStream* pInputStream = new Common::FileInputStream(path.c_str());

        if (0 != image.Load(pInputStream))
        {
            delete pInputStream;
            LOG_ERROR("Failed to open '%s'.", mName);
            return false;
        }
        delete pInputStream;
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
            if (image.LoadFromFile(path) != Result::OK)
            {
                LOG_ERROR("Failed to open '%s'.", mName);
                return false;
            }
            LOG_SUCCESS("File '%s' loaded.", mName);
        }
        else
        {
            if (image.LoadFromBuffer(buffer.GetData(), buffer.GetSize()) != Result::OK)
            {
                LOG_ERROR("Failed to open '%s'.", mName);
                return false;
            }
            LOG_SUCCESS("File '%s' loaded from PAK file.", mName);
        }
        */

        //mTex = g_pRenderer->CreateTexture(&image, true);
        if (CreateFromImage(image) != Result::OK)
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
        mOnLoad(this, mUserPtr);

    LOG_INFO("Unloading texture '%s'...", mName);
    Release();
}



//Load the texture from a stream (supported formats: BMP, JPEG and PNG).
Result Texture::CreateFromStream(Common::InputStream* pStream)
{
    return Result::OK;
}

//Create the texture from a custom memory buffer.
Result Texture::CreateFromImage(const Common::Image& image)
{
    Release();

    mTex = g_pRenderer->CreateTexture();
    if (mTex == nullptr)
    {
        LOG_ERROR("Failed to create texture object for '%s'.", mName);
        return Result::Error;
    }

    if (mTex->FromImage(image) != Result::OK)
    {
        LOG_ERROR("Failed to load image to texture object for '%s'.", mName);
        return Result::Error;
    }

    return Result::OK;
}


using namespace Render;

// Create renderable texture, for example to show monitor screen with a view from a camera.
IRenderTarget* Texture::CreateRendertarget(uint32 width, uint32 height, Common::ImageFormat format)
{
    Release();

    IRenderTarget* pRT = g_pRenderer->CreateRenderTarget();
    if (!pRT)
    {
        LOG_ERROR("Failed to allocate texture object");
        return nullptr;
    }

    if (pRT->Init(width, height, nullptr, true) != 0)
    {
        LOG_ERROR("Failed to create render target");
        return nullptr;
    }

    mTex = pRT;
    return pRT;
}

IRendererTexture* Texture::GetRendererTexture() const
{
    return mTex;
}

} // namespace Resource
} // namespace NFE
