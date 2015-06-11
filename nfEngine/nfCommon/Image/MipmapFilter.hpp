/**
 * @file
 * @author mkkulagowski
 * @brief  Mipmap class declaration.
 */

#pragma once

#include "Mipmap.hpp"

namespace NFE {
namespace Common {

class MipmapFilter
{
protected:
    Mipmap* mMipmap;
    
public:
    MipmapFilter(Mipmap* mipmap);
    ~MipmapFilter();
    MipmapFilter(const MipmapFilter& other) = delete;
    MipmapFilter(MipmapFilter&& other) = delete;
    
    virtual Vector Calc(int x, int y);
    
    // needs to be deleted afterwards!
    static MipmapFilter* CreateFilter(int filter, Mipmap* mmap);
}


class BoxFilter : MipmapFilter
{
    virtual Vector Calc(int x, int y);
}

} // Common
} // NFE
