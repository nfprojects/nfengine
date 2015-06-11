/**
 * @file
 * @author mkkulagowski
 * @brief  Mipmap class declaration.
 */

#pragma once

#include "Mipmap.hpp"

namespace NFE {
namespace Common {

//class Mipmap;
//class Vector;

class MipmapFilter
{
protected:
    Mipmap* mMipmap;
    
public:
    MipmapFilter(Mipmap* mipmap);
    ~MipmapFilter();
    MipmapFilter(const MipmapFilter& other) = delete;
    MipmapFilter(MipmapFilter&& other) = delete;
    
	// Calculates Vector using created filter. Default is box filter
    virtual Vector Calc(uint32 x, uint32 y);
    
    // needs to be deleted afterwards!
    static MipmapFilter* CreateFilter(int filter, Mipmap* mmap);
};


class GammaCorrectedFilter : public MipmapFilter
{
public:
	GammaCorrectedFilter(Mipmap* mipmap);
	virtual Vector Calc(uint32 x, uint32 y);
};

} // namespace Common
} // namespace NFE
