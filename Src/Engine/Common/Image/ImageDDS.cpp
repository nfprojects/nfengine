/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  ImageDDS class definition.
 */


#include "PCH.hpp"
#include "ImageDDS.hpp"
#include "Image.hpp"
#include "Logger/Logger.hpp"
#include "Utils/Stream/InputStream.hpp"
#include "Reflection/ReflectionClassDefine.hpp"


NFE_DEFINE_POLYMORPHIC_CLASS(NFE::Common::ImageDDS)
    NFE_CLASS_PARENT(NFE::Common::ImageType)
NFE_END_DEFINE_CLASS()

#define DDS_MAGIC_NUMBER    0x20534444

#define DDSD_CAPS   0x1
#define DDSD_HEIGHT 0x2
#define DDSD_WIDTH  0x4
#define DDSD_PITCH  0x8
#define DDSD_PIXELFORMAT    0x1000
#define DDSD_MIPMAPCOUNT    0x20000
#define DDSD_LINEARSIZE 0x80000
#define DDSD_DEPTH  0x800000

#define DDPF_ALPHAPIXELS 0x00000001
#define DDPF_ALPHA 0x00000002
#define DDPF_FOURCC 0x00000004
#define DDPF_PALETTEINDEXED8 0x00000020
#define DDPF_RGB 0x00000040
#define DDPF_LUMINANCE 0x00020000

#define ID_DXT1   0x31545844
#define ID_DXT3   0x33545844
#define ID_DXT5   0x35545844

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((NFE::uint32)(NFE::uint8)(ch0)        | \
                                       ((NFE::uint32)(NFE::uint8)(ch1) << 8)  | \
                                       ((NFE::uint32)(NFE::uint8)(ch2) << 16) | \
                                       ((NFE::uint32)(NFE::uint8)(ch3) << 24))
#endif /* defined(MAKEFOURCC) */

#define ISBITMASK(r, g, b, a) (ddpf.dwRBitMask == r &&   \
                               ddpf.dwGBitMask == g &&   \
                               ddpf.dwBBitMask == b &&   \
                               ddpf.dwAlphaBitMask == a)


namespace NFE {
namespace Common {

namespace {

struct DDS_PIXELFORMAT
{
    unsigned int    dwSize;
    unsigned int    dwFlags;
    unsigned int    dwFourCC;
    unsigned int    dwRGBBitCount;
    unsigned int    dwRBitMask;
    unsigned int    dwGBitMask;
    unsigned int    dwBBitMask;
    unsigned int    dwAlphaBitMask;
};

struct DDS_header
{
    unsigned int    dwMagic;
    unsigned int    dwSize;
    unsigned int    dwFlags;
    unsigned int    dwHeight;
    unsigned int    dwWidth;
    unsigned int    dwPitchOrLinearSize;
    unsigned int    dwDepth;
    unsigned int    dwMipMapCount;
    unsigned int    dwReserved1[11];

    //  DDPIXELFORMAT
    DDS_PIXELFORMAT sPixelFormat;

    //  DDCAPS2
    struct
    {
        unsigned int    dwCaps1;
        unsigned int    dwCaps2;
        unsigned int    dwDDSX;
        unsigned int    dwReserved;
    } sCaps;

    unsigned int dwReserved2;
};

static ImageFormat DDSGetFormat(const DDS_PIXELFORMAT& ddpf)
{
    if (ddpf.dwFlags & DDPF_RGB)
    {
        // Note that sRGB formats are written using the "DX10" extended header

        switch (ddpf.dwRGBBitCount)
        {
            case 32:
                if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
                    return ImageFormat::RGBA_UByte; //DXGI_FORMAT_R8G8B8A8_UNORM;

                if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
                    // Only 32-bit color channel format in D3D9 was R32F
                    return ImageFormat::R_Float; //DXGI_FORMAT_R32_FLOAT

                //if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
                //  return DXGI_FORMAT_B8G8R8A8_UNORM;

                //if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
                //    return DXGI_FORMAT_B8G8R8X8_UNORM;

                // No DXGI format maps to ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000) aka D3DFMT_X8B8G8R8

                // Note that many common DDS reader/writers (including D3DX) swap the
                // the RED/BLUE masks for 10:10:10:2 formats. We assumme
                // below that the 'backwards' header mask is being used since it is most
                // likely written by D3DX. The more robust solution is to use the 'DX10'
                // header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

                // For 'correct' writers, this should be 0x000003ff, 0x000ffc00, 0x3ff00000 for RGB data
                //if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
                //    return DXGI_FORMAT_R10G10B10A2_UNORM;

                // No DXGI format maps to ISBITMASK(0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000) aka D3DFMT_A2R10G10B10
                //if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
                //    return DXGI_FORMAT_R16G16_UNORM;

                break;

            case 24:
                // No 24bpp DXGI formats aka D3DFMT_R8G8B8
                break;

            case 16:
                /*
                if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
                    return DXGI_FORMAT_B5G5R5A1_UNORM;

                if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
                    return DXGI_FORMAT_B5G6R5_UNORM;

                if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
                    return DXGI_FORMAT_B4G4R4A4_UNORM;
                    */

                break;
        }
    }
    else if (ddpf.dwFlags & DDPF_LUMINANCE)
    {
        if (8 == ddpf.dwRGBBitCount)
        {
            if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
                return ImageFormat::A_UByte; //DXGI_FORMAT_R8_UNORM
        }
    }
    else if (ddpf.dwFlags & DDPF_ALPHA)
    {
        if (8 == ddpf.dwRGBBitCount)
            return ImageFormat::A_UByte; //DXGI_FORMAT_A8_UNORM
    }
    else if (ddpf.dwFlags & DDPF_FOURCC)
    {
        if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.dwFourCC)
            return ImageFormat::BC1; //DXGI_FORMAT_BC1_UNORM

        if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.dwFourCC)
            return ImageFormat::BC2; //DXGI_FORMAT_BC2_UNORM

        if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.dwFourCC)
            return ImageFormat::BC3; //DXGI_FORMAT_BC3_UNORM


        // While pre-multiplied alpha isn't directly supported by the DXGI formats,
        // they are basically the same as these BC formats so they can be mapped
        if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.dwFourCC)
            return ImageFormat::BC2; //DXGI_FORMAT_BC2_UNORM

        if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.dwFourCC)
            return ImageFormat::BC3; //DXGI_FORMAT_BC3_UNORM

        if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.dwFourCC)
            return ImageFormat::BC4; //DXGI_FORMAT_BC4_UNORM

        if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.dwFourCC)
            return ImageFormat::BC4; //DXGI_FORMAT_BC4_UNORM

        if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.dwFourCC)
            return ImageFormat::BC4; //DXGI_FORMAT_BC4_SNORM

        if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.dwFourCC)
            return ImageFormat::BC5; //DXGI_FORMAT_BC5_UNORM

        if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.dwFourCC)
            return ImageFormat::BC5; //DXGI_FORMAT_BC5_UNORM

        if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.dwFourCC)
            return ImageFormat::BC5; //DXGI_FORMAT_BC1_SNORM


        // Check for D3DFORMAT enums being set here
        switch (ddpf.dwFourCC)
        {
            case 114: // D3DFMT_R32F
                return ImageFormat::R_Float; //DXGI_FORMAT_R32_FLOAT
            case 116: // D3DFMT_A32B32G32R32F
                return ImageFormat::RGBA_Float; //DXGI_FORMAT_R32G32B32A32_FLOAT
        }
    }

    return ImageFormat::Unknown;
}

} // namespace

//////////////////////////////////////////////////////////////////////////

StringView ImageDDS::GetName() const
{
    const StringView name("DDS");
    return name;
}

bool ImageDDS::Check(InputStream* stream)
{
    uint32 signature = 0;
    stream->Seek(0);
    if (sizeof(signature) < stream->Read(&signature, sizeof(signature)))
    {
        NFE_LOG_ERROR("Could not read signature from the stream.");
        return false;
    }

    stream->Seek(0);

    return signature == 0x20534444;
}

bool ImageDDS::Load(Image* img, InputStream* stream)
{
    // read header
    DDS_header header;
    if (stream->Read(&header, sizeof(header)) != sizeof(header))
        return false;

    //check magic number
    if (header.dwMagic != DDS_MAGIC_NUMBER)
        return false;

    int width = header.dwWidth;
    int height = header.dwHeight;
    int initWidth = header.dwWidth;
    int initHeight = header.dwHeight;
    int numMipmaps = header.dwMipMapCount;
    if (numMipmaps < 1) numMipmaps = 1;
    if (numMipmaps > 32) return 1;

    ImageFormat format = DDSGetFormat(header.sPixelFormat);
    if (format == ImageFormat::Unknown)
        return false;

    DynArray<Mipmap> mipmaps;
    for (int i = 0; i < numMipmaps; i++)
    {
        //keep mipmap size > 0
        if (width == 0) width = 1;
        if (height == 0) height = 1;


        size_t dataSize = ((width + 3) / 4) * ((height + 3) / 4) * 16 * BitsPerPixel(format) / 8;
        std::unique_ptr<uint8[]> mipmapData(new (std::nothrow) uint8[dataSize]);
        if (!mipmapData.get())
        {
            NFE_LOG_ERROR("Allocating memory for loading DDS image failed.");
            img->Release();
            return false;
        }

        if (stream->Read(mipmapData.get(), dataSize) != dataSize)
        {
            NFE_LOG_ERROR("Reading mipmap data failed.");
            return false;
        }

        mipmaps.PushBack(Mipmap(mipmapData.get(), width, height, dataSize));

        width /= 2;
        height /= 2;
    }

    return img->SetData(std::move(mipmaps), initWidth, initHeight, format);
}

bool ImageDDS::Save(Image*, OutputStream*)
{
    // TODO implement

    return false;
}

} // namespace Common
} // namespace NFE