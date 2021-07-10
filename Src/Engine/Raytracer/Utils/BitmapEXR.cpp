#include "PCH.h"
#include "Bitmap.h"
#include "../Common/Containers/DynArray.hpp"
#include "../Common/Math/Half.hpp"
#include "tinyexr/tinyexr.h"

namespace NFE {
namespace RT {

using namespace Common;
using namespace Math;

bool Bitmap::LoadEXR(FILE* file, const char* path)
{
    NFE_UNUSED(file);

    // 1. Read EXR version.
    EXRVersion exrVersion;

    int ret = ParseEXRVersionFromFile(&exrVersion, path);
    if (ret != 0)
    {
        NFE_LOG_ERROR("Invalid EXR file: %s", path);
        return false;
    }

    if (exrVersion.multipart)
    {
        NFE_LOG_ERROR("Multipart EXR are not supported: %s", path);
        return false;
    }

    // 2. Read EXR header
    EXRHeader exrHeader;
    InitEXRHeader(&exrHeader);

    const char* err = NULL;
    ret = ParseEXRHeaderFromFile(&exrHeader, &exrVersion, path, &err);
    if (ret != 0)
    {
        NFE_LOG_ERROR("Parse EXR error: %s", err);
        FreeEXRErrorMessage(err);
        return ret;
    }

    EXRImage exrImage;
    InitEXRImage(&exrImage);

    ret = LoadEXRImageFromFile(&exrImage, &exrHeader, path, &err);
    if (ret != 0)
    {
        NFE_LOG_ERROR("Load EXR error: %s", err);
        FreeEXRErrorMessage(err);
        return ret;
    }

    if (!exrImage.images)
    {
        NFE_LOG_ERROR("Tiled EXR are not supported: %s", path);
        FreeEXRImage(&exrImage);
        return false;
    }

    InitData initData;
    initData.width = exrImage.width;
    initData.height = exrImage.height;

    size_t numInvalidValues = 0;
    size_t numNegativeValues = 0;

    const auto validateFloatValue = [&numInvalidValues, &numNegativeValues](float& val)
    {
        if (!IsValid(val))
        {
            numInvalidValues++;
            val = 0.0f;
        }

        if (val < 0.0f)
        {
            numNegativeValues++;
            val = 0.0f;
        }
    };

    const auto validateHalfValue = [&numInvalidValues, &numNegativeValues](Half& val)
    {
        if (!val.IsValid())
        {
            numInvalidValues++;
            val = 0;
        }

        if (val.components.sign)
        {
            numNegativeValues++;
            val = 0;
        }
    };

    if (exrHeader.num_channels == 3)
    {
        const bool sameFormat = exrHeader.pixel_types[0] == exrHeader.pixel_types[1] && exrHeader.pixel_types[0] == exrHeader.pixel_types[2];
        if (!sameFormat)
        {
            NFE_LOG_ERROR("Unsupported EXR format. All channels must be of the same type");
            goto exrImageError;
        }

        if (exrHeader.pixel_types[0] == TINYEXR_PIXELTYPE_FLOAT)
        {
            initData.format = Format::R32G32B32_Float;
            if (!Init(initData))
            {
                goto exrImageError;
            }

            for (size_t y = 0; y < (size_t)exrImage.height; ++y)
            {
                float* typedData = reinterpret_cast<float*>(mData + (size_t)GetStride() * y);
                for (size_t x = 0; x < (size_t)exrImage.width; ++x)
                {
                    const size_t index = y * exrImage.width + x;
                    typedData[3 * x    ] = reinterpret_cast<const float*>(exrImage.images[2])[index];
                    typedData[3 * x + 1] = reinterpret_cast<const float*>(exrImage.images[1])[index];
                    typedData[3 * x + 2] = reinterpret_cast<const float*>(exrImage.images[0])[index];
                    validateFloatValue(typedData[3 * x    ]);
                    validateFloatValue(typedData[3 * x + 1]);
                    validateFloatValue(typedData[3 * x + 2]);
                }
            }
        }
        else if (exrHeader.pixel_types[0] == TINYEXR_PIXELTYPE_HALF)
        {
            initData.format = Format::R16G16B16_Half;
            if (!Init(initData))
            {
                goto exrImageError;
            }

            for (size_t y = 0; y < (size_t)exrImage.height; ++y)
            {
                Half* typedData = reinterpret_cast<Half*>(mData + GetStride() * y);
                for (size_t x = 0; x < (size_t)exrImage.width; ++x)
                {
                    const size_t index = y * exrImage.width + x;
                    typedData[3 * x    ] = reinterpret_cast<const Half*>(exrImage.images[2])[index];
                    typedData[3 * x + 1] = reinterpret_cast<const Half*>(exrImage.images[1])[index];
                    typedData[3 * x + 2] = reinterpret_cast<const Half*>(exrImage.images[0])[index];
                    validateHalfValue(typedData[3 * x    ]);
                    validateHalfValue(typedData[3 * x + 1]);
                    validateHalfValue(typedData[3 * x + 2]);
                }
            }
        }
        else
        {
            NFE_LOG_ERROR("Unsupported EXR format: %i", exrHeader.pixel_types[0]);
            goto exrImageError;
        }
    }
    else if (exrHeader.num_channels == 4)
    {
        const bool sameFormat = exrHeader.pixel_types[0] == exrHeader.pixel_types[1]
            && exrHeader.pixel_types[0] == exrHeader.pixel_types[2]
            && exrHeader.pixel_types[0] == exrHeader.pixel_types[3];
        if (!sameFormat)
        {
            NFE_LOG_ERROR("Unsupported EXR format. All channels must be of the same type");
            goto exrImageError;
        }

        if (exrHeader.pixel_types[0] == TINYEXR_PIXELTYPE_FLOAT)
        {
            initData.format = Format::R32G32B32A32_Float;
            if (!Init(initData))
            {
                goto exrImageError;
            }

            for (size_t y = 0; y < (size_t)exrImage.height; ++y)
            {
                float* typedData = reinterpret_cast<float*>(mData + GetStride() * y);
                for (size_t x = 0; x < (size_t)exrImage.width; ++x)
                {
                    const size_t index = y * exrImage.width + x;
                    typedData[4 * x    ] = reinterpret_cast<const float*>(exrImage.images[3])[index];
                    typedData[4 * x + 1] = reinterpret_cast<const float*>(exrImage.images[2])[index];
                    typedData[4 * x + 2] = reinterpret_cast<const float*>(exrImage.images[1])[index];
                    typedData[4 * x + 3] = reinterpret_cast<const float*>(exrImage.images[0])[index];
                    validateFloatValue(typedData[4 * x    ]);
                    validateFloatValue(typedData[4 * x + 1]);
                    validateFloatValue(typedData[4 * x + 2]);
                    validateFloatValue(typedData[4 * x + 3]);
                }
            }
        }
        else if (exrHeader.pixel_types[0] == TINYEXR_PIXELTYPE_HALF)
        {
            initData.format = Format::R16G16B16A16_Half;
            if (!Init(initData))
            {
                goto exrImageError;
            }

            for (size_t y = 0; y < (size_t)exrImage.height; ++y)
            {
                Half* typedData = reinterpret_cast<Half*>(mData + GetStride() * y);
                for (size_t x = 0; x < (size_t)exrImage.width; ++x)
                {
                    const size_t index = y * exrImage.width + x;
                    typedData[4 * x    ] = reinterpret_cast<const Half*>(exrImage.images[3])[index];
                    typedData[4 * x + 1] = reinterpret_cast<const Half*>(exrImage.images[2])[index];
                    typedData[4 * x + 2] = reinterpret_cast<const Half*>(exrImage.images[1])[index];
                    typedData[4 * x + 3] = reinterpret_cast<const Half*>(exrImage.images[0])[index];
                    validateHalfValue(typedData[4 * x    ]);
                    validateHalfValue(typedData[4 * x + 1]);
                    validateHalfValue(typedData[4 * x + 2]);
                    validateHalfValue(typedData[4 * x + 3]);
                }
            }
        }
        else
        {
            NFE_LOG_ERROR("Unsupported EXR format: %i", exrHeader.pixel_types[0]);
            goto exrImageError;
        }
    }
    else
    {
        NFE_LOG_ERROR("Unsupported EXR format.", path);
        goto exrImageError;
    }

    if (numInvalidValues)
    {
        NFE_LOG_WARNING("EXR image contains %zu invalid values", numInvalidValues);
    }

    if (numNegativeValues)
    {
        NFE_LOG_WARNING("EXR image contains %zu invalid values", numNegativeValues);
    }

    // 4. Free image data
    FreeEXRImage(&exrImage);
    return true;

exrImageError:
    FreeEXRImage(&exrImage);
    return false;
}

bool Bitmap::SaveEXR(const char* path, const float exposure) const
{
    if (GetDepth() > 1)
    {
        NFE_LOG_ERROR("Bitmap::SaveEXR: Cannot save 3D texture as BMP file");
        return false;
    }

    if (GetWidth() == 0 || GetHeight() == 0)
    {
        NFE_LOG_ERROR("Bitmap::SaveEXR: Cannot save empty texture");
        return false;
    }

    if (mFormat != Format::R32G32B32_Float)
    {
        NFE_LOG_ERROR("Bitmap::SaveEXR: Unsupported format: %s", FormatToString(mFormat));
        return false;
    }

    // TODO support more types

    const Vec3f* data = reinterpret_cast<const Vec3f*>(mData);

    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    image.num_channels = 3;

    DynArray<float> images[3];
    images[0].Resize(GetWidth() * GetHeight());
    images[1].Resize(GetWidth() * GetHeight());
    images[2].Resize(GetWidth() * GetHeight());

    // Split RGBRGBRGB... into R, G and B layer
    const uint32 numPixels = GetWidth() * GetHeight();
    for (uint32 i = 0; i < numPixels; i++)
    {
        images[0][i] = exposure * data[i].x;
        images[1][i] = exposure * data[i].y;
        images[2][i] = exposure * data[i].z;
    }

    float* image_ptr[3];
    image_ptr[0] = images[2].Data(); // B
    image_ptr[1] = images[1].Data(); // G
    image_ptr[2] = images[0].Data(); // R

    image.images = (unsigned char**)image_ptr;
    image.width = GetWidth();
    image.height = GetHeight();

    header.compression_type = TINYEXR_COMPRESSIONTYPE_PIZ;
    header.num_channels = 3;
    header.channels = (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo) * header.num_channels);

    // Must be (A)BGR order, since most of EXR viewers expect this channel order.
    {
        strcpy(header.channels[0].name, "B");
        strcpy(header.channels[1].name, "G");
        strcpy(header.channels[2].name, "R");
    }

    header.pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++)
    {
        header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
        header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of output image to be stored in .EXR
    }

    const char* err = nullptr;
    int ret = SaveEXRImageToFile(&image, &header, path, &err);
    if (ret != TINYEXR_SUCCESS)
    {
        NFE_LOG_ERROR("Failed to save EXR file '%s': %s", path, err);
        FreeEXRErrorMessage(err);

        free(header.channels);
        free(header.pixel_types);
        free(header.requested_pixel_types);

        return ret;
    }

    NFE_LOG_INFO("Image file '%s' written successfully", path);

    free(header.channels);
    free(header.pixel_types);
    free(header.requested_pixel_types);

    return true;
}

} // namespace RT
} // namespace NFE
