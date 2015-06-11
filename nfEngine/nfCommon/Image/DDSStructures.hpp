/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Structures for DDS files.
 */

#pragma once
#include "../nfCommon.hpp"

namespace NFE {
namespace Common {

struct BC1
{
	// 5:6:5 colors
	uchar color1[2];
	uchar color4[2];

	// 16 2bit color indices
	uchar indices[4];
};

struct BC2
{
    // 16 4bit alphas
    uchar alpha[8];

	// 5:6:5 colors
	uchar color1[2];
	uchar color4[2];

	// 16 2bit color indices
	uchar indices[4];
};

struct BC3
{
    // 2 8bit alphas
    uchar alpha0;
    uchar alpha1;

    // 16 3bit alpha indices
    uchar alphaIndices[6];

	// 5:6:5 colors
	uchar color1[2];
	uchar color4[2];

	// 16 2bit color indices
	uchar indices[4];
};

} // namespace Common
} // namespace NFE