#include "PCH.h"
#include "Texture.h"

namespace NFE {
namespace RT {

ITexture::~ITexture() = default;

bool ITexture::MakeSamplable()
{
    return true;
}

bool ITexture::IsSamplable() const
{
    return true;
}

} // namespace RT
} // namespace NFE
