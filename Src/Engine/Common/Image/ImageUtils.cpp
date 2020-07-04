/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Image class definitions.
 */

#include "PCH.hpp"
#include "ImageUtils.hpp"
#include "ImageType.hpp"
#include "Mipmap.hpp"
#include "BlockCompression.hpp"


namespace {

// TODO After BC6H-BC7 implementation refactor gDDSSrcFormat
/**
 * Source format, from which DDS conversion is made
 */
const NFE::Common::ImageFormat gDDSSrcFormat = NFE::Common::ImageFormat::RGBA_Float;
}

namespace NFE {
namespace Common {
namespace ImageUtils {
    bool Load(InputStream* stream, Image* dst, ImageFormat destFormat)
    {
        for (const auto& imageType : ImageType::GetTypes())
        {
            if (imageType->Check(stream))
            {
                const auto res = imageType->Load(dst, stream);
                if (res && destFormat != ImageFormat::Unknown)
                    return Convert(dst, destFormat);
                else
                    return res;
            }
        }

        NFE_LOG_ERROR("Stream was not recognized as any of the usable file formats.");
        return false;
    }

    UniquePtr<Image> Load(InputStream* stream, ImageFormat destFormat)
    {
        UniquePtr<Image> img = MakeUniquePtr<Image>();
        if (Load(stream, img.Get(), destFormat))
            return img;
        return UniquePtr<Image>();
    }

    bool Empty(const uint32 width, const uint32 height, const ImageFormat format, Image* dst)
    {
        UniquePtr<uint8[]> data = MakeUniquePtr<uint8[]>(width * height * BitsPerPixel(format) / 8);
        return dst->SetData(data.Get(), width, height, format);
    }

    UniquePtr<Image> Empty(const uint32 width, const uint32 height, const ImageFormat format)
    {
        UniquePtr<Image> img = MakeUniquePtr<Image>();
        if (Empty(width, height, format, img.Get()))
            return img;
        return UniquePtr<Image>();
    }

   

    bool DecompressDDS(Image* img)
    {
        const uint32 w = img->GetWidth();
        const uint32 h = img->GetHeight();
        UniquePtr<Image> decompressed = Empty(w, h, gDDSSrcFormat);
        DecodeBCnFunctor decodePixelFunc;
        switch (img->GetFormat())
        {
        case ImageFormat::BC1:
            decodePixelFunc = NFE::DecodeBC1;
            break;
        case ImageFormat::BC2:
            decodePixelFunc = NFE::DecodeBC2;
            break;
        case ImageFormat::BC3:
            decodePixelFunc = NFE::DecodeBC3;
            break;
        case ImageFormat::BC4:
            decodePixelFunc = NFE::DecodeBC4;
            break;
        case ImageFormat::BC5:
            decodePixelFunc = NFE::DecodeBC5;
            break;
        default:
            NFE_LOG_WARNING("BC Decompression cannot be done for %s pixel format.",
                FormatToStr(img->GetFormat()));
            return false;
        }

        for (uint32 y = 0; y < h; ++y)
        {
            for (uint32 x = 0; x < w; ++x)
            {
                const Color pixelValue = decodePixelFunc(reinterpret_cast<const uint8*>(img->GetData()), x, y, w);
                decompressed->SetTexel(pixelValue, x, y);
            }
        }

        *img = std::move(*decompressed.ReleaseOwnership());
        return true;
    }

    bool ConvertActual(Image* img, ImageFormat destFormat)
    {
        DynArray<Mipmap> newMipmaps;
        newMipmaps.Reserve(img->GetMipmapsNum());

        for (uint32 i = 0; i < img->GetMipmapsNum(); i++)
        {
            uint32 width = img->GetMipmap(i)->GetWidth();
            uint32 height = img->GetMipmap(i)->GetHeight();
            size_t dataSize = width * height * BitsPerPixel(img->GetFormat()) / 8;
            // Make empty data for the new mipmap so it allocates the space needed
            UniquePtr<uint8[]> data = MakeUniquePtr<uint8[]>(dataSize);
            newMipmaps.PushBack(Mipmap(data.Get(), width, height, dataSize));

            Math::Vec4f tmp;

            for (uint32 y = 0; y < height; y++)
            {
                for (uint32 x = 0; x < width; x++)
                {
                    tmp = img->GetMipmap(i)->GetTexel(x, y, img->GetFormat());
                    newMipmaps.Back().SetTexel(tmp, x, y, destFormat);
                }
            }
        }

        return img->SetData(std::move(newMipmaps), img->GetWidth(), img->GetHeight(), destFormat);
    }

    bool Convert(Image* src, ImageFormat destFormat)
    {
        if (src->GetData() == 0)
        {
            NFE_LOG_WARNING("Tried to convert pixel format of an empty image.");
            return false;
        }

        if (destFormat == src->GetFormat())
            return true;

        if (src->GetFormat() == ImageFormat::BC6H || src->GetFormat() == ImageFormat::BC7)
        {
            NFE_LOG_ERROR("Conversion from BC6H-BC7 pixel formats is currently not supported.");
            return false;
        }

        if (IsSupportedBC(destFormat) || destFormat == ImageFormat::BC6H || destFormat == ImageFormat::BC7)
        {
            NFE_LOG_ERROR("Conversion to BCn pixel formats is currently not supported.");
            return false;
        }

        // Invalid format
        if (src->GetFormat() == ImageFormat::Unknown || destFormat == ImageFormat::Unknown)
        {
            NFE_LOG_ERROR("Trying to convert invalid pixel format.");
            return false;
        }

        if (IsSupportedBC(src->GetFormat()))
        {
            if (!DecompressDDS(src))
            {
                NFE_LOG_ERROR("BC decompression failed.");
                return false;
            }
            GenerateMipmaps(src, MipmapFilter::Box, src->GetMipmapsNum() - 1);
        }

        return ConvertActual(src, destFormat);
    }

    Color FilterBox(const Mipmap* mip, const uint32 x, const uint32 y, const ImageFormat fmt)
    {
        Color a = mip->GetTexel(2 * x, 2 * y, fmt);
        Color b = mip->GetTexel(2 * x + 1, 2 * y, fmt);
        Color c = mip->GetTexel(2 * x, 2 * y + 1, fmt);
        Color d = mip->GetTexel(2 * x + 1, 2 * y + 1, fmt);

        return ((a + b) + (c + d)) * 0.25f;
    }

    Color FilterGammaCorrected(const Mipmap* mip, const uint32 x, const uint32 y, const ImageFormat fmt)
    {
        Color a = mip->GetTexel(2 * x, 2 * y, fmt);
        Color b = mip->GetTexel(2 * x + 1, 2 * y, fmt);
        Color c = mip->GetTexel(2 * x, 2 * y + 1, fmt);
        Color d = mip->GetTexel(2 * x + 1, 2 * y + 1, fmt);
        a *= a;
        b *= b;
        c *= c;
        d *= d;

        return Math::Vec4f::Sqrt(((a + b) + (c + d)) * 0.25f);
    }

    using MipmapFilterFunctor = Color(*)(const Mipmap*, const uint32, const uint32, const ImageFormat);
    bool GenerateMipmapsActual(Image* img, MipmapFilter filterType, uint32 num)
    {
        // Resolve what filter should be used
        MipmapFilterFunctor filter = nullptr;
        switch (filterType)
        {
        case MipmapFilter::GammaCorrectedLinear:
            filter = &FilterGammaCorrected;
            break;

        case MipmapFilter::Box:
            filter = &FilterBox;
            break;

        default:
            NFE_LOG_ERROR("Unknown filter specified for mipmap generation: %s.", FilterToStr(filterType));
            return false;
        }

        // TODO  NEEDS CHANGE
        auto isPowerOfTwo = [](int x) {return !(x == 0) && !(x & (x - 1)); };
        if (isPowerOfTwo(img->GetWidth()) || isPowerOfTwo(img->GetHeight()))
            NFE_LOG_WARNING("No support for non-power-of-2 !");

        // Generate mipmaps main loop
        DynArray<Mipmap> mips;
        mips.PushBack(*img->GetMipmap());
        for (uint32 i = 0; i < num; i++)
        {
            const Mipmap& mipmap = mips.Back();

            // Calculate the size of the new mipmap
            uint32 nextMipWidth = (mipmap.GetWidth() / 2) + (mipmap.GetWidth() % 2);
            uint32 nextMipHeight = (mipmap.GetHeight() / 2) + (mipmap.GetHeight() % 2);
            if (nextMipWidth == 0)
                nextMipWidth = 1;
            if (nextMipHeight == 0)
                nextMipHeight = 1;
            size_t dataSize = nextMipWidth * nextMipHeight * BitsPerPixel(img->GetFormat());

            // Make empty data for the new mipmap so it allocates the space needed
            UniquePtr<uint8[]> data = MakeUniquePtr<uint8[]>(dataSize / sizeof(uint8));
            if (!data.Get())
            {
                NFE_LOG_ERROR("Allocating memory for mipmap generation failed.");
                return false;
            }

            Mipmap nextMip(data.Get(), nextMipWidth, nextMipHeight, dataSize);

            // Iterate through whole mipmap and acquire its texels
            for (uint32 y = 0; y < nextMipHeight; y++)
            {
                for (uint32 x = 0; x < nextMipWidth; x++)
                {
                    Color outputTexel = (*filter)(&mipmap, x, y, img->GetFormat());
                    nextMip.SetTexel(outputTexel, x, y, img->GetFormat());
                }
            }
            mips.PushBack(std::move(nextMip));

            if (nextMipWidth == 1 && nextMipHeight == 1)
                break;
        }

        return img->SetData(std::move(mips), img->GetWidth(), img->GetHeight(), img->GetFormat());
    }

    bool GenerateMipmaps(Image* img, MipmapFilter filterType, uint32 num)
    {
        // Empty image
        if (!img->GetData())
        {
            NFE_LOG_WARNING("Tried to generate mMipmaps of an empty image");
            return false;
        }

        if (IsSupportedBC(img->GetFormat()) || img->GetFormat() == ImageFormat::BC6H
            || img->GetFormat() == ImageFormat::BC7)
        {
            NFE_LOG_ERROR("Generating mipmaps for BCn formats unsupported. Convert to other format beforehand.");
            return false;
        }

        if (img->GetFormat() == ImageFormat::Unknown)
        {
            NFE_LOG_ERROR("Invalid pixel format");
            return false;
        }

        return GenerateMipmapsActual(img, filterType, num);
    }

    bool Grayscale(Image* img)
    {
        if (img->GetFormat() != ImageFormat::RGB_UByte
            && img->GetFormat() != ImageFormat::RGBA_UByte
            && img->GetFormat() != ImageFormat::RGBA_Float)
        {
            NFE_LOG_ERROR("Conversion to grayscale is not available for image format: %s.", FormatToStr(img->GetFormat()));
            return false;
        }

        // These are common values for luminance grayscale transformation
        Color luminanceMask(0.21f, 0.72f, 0.07f, 1.0f);

        DynArray<Mipmap> newMipmaps;
        newMipmaps.Reserve(img->GetMipmapsNum());
        for (uint32 i = 0; i < img->GetMipmapsNum(); i++)
        {
            uint32 width = img->GetMipmap(i)->GetWidth();
            uint32 height = img->GetMipmap(i)->GetHeight();

            Color tmp;
            size_t dataSize = width * height * BitsPerPixel(img->GetFormat()) / 8;
            // Make empty data for the new mipmap so it allocates the space needed
            UniquePtr<uint8[]> data = MakeUniquePtr<uint8[]>(dataSize);
            newMipmaps.PushBack(Mipmap(data.Get(), width, height, dataSize));

            for (uint32 y = 0; y < height; y++)
            {
                for (uint32 x = 0; x < width; x++)
                {
                    // Get texel, so we can calculate dot product and use its alpha
                    tmp = img->GetMipmap(i)->GetTexel(x, y, img->GetFormat());

                    // Calculate grayscale value using luminance transformation
                    float grayVal = Math::Vec4f::Dot3(tmp, luminanceMask);

                    // Overwrite only color values, leaving alpha as it was
                    newMipmaps.Back().SetTexel(Color(grayVal, grayVal, grayVal, tmp.f[3]),
                        x, y, img->GetFormat());
                }
            }
        }
        return img->SetData(std::move(newMipmaps), img->GetWidth(), img->GetHeight(), img->GetFormat());
    }

} // namespace ImageUtils
} // namespace Common
} // namespace NFE
