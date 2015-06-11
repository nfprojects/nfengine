
#include "../PCH.hpp"
#include "MipmapFilter.hpp"

namespace NFE {
namespace Common {

MipmapFilter::MipmapFilter(Mipmap* mipmap)
	: mMipmap(mipmap)
{
}

MipmapFilter::~MipmapFilter()
{
	mMipmap = nullptr;
}

MipmapFilter* MipmapFilter::CreateFilter(int filter, Mipmap* mmap)
{
    switch(filter)
    {
        case 0:
            return static_cast<MipmapFilter*>(new (std::nothrow) BoxFilter(mmap));
    }
    
    return nullptr;
}

BoxFilter::BoxFilter(Mipmap* mipmap)
	: MipmapFilter(mipmap)
{
}

Vector MipmapFilter::Calc(uint32 x, uint32 y)
{
    Vector a = mMipmap->GetTexel(2 * x,     2 * y    );
    Vector b = mMipmap->GetTexel(2 * x + 1, 2 * y    );
    Vector c = mMipmap->GetTexel(2 * x,     2 * y + 1);
    Vector d = mMipmap->GetTexel(2 * x + 1, 2 * y + 1);

    return ((a + b) + (c + d)) * 0.25f;
}

} // namespace Common
} // namespace NFE
