#pragma once
#include "../nfCommon.hpp"

namespace NFE {
namespace Common {

struct DDSStruct
{
    // 5:6:5 colors
    uchar color1[2];
    uchar color4[2];
    // 16 2bit color indices
    uchar indices[4];
};

struct BC1 : DDSStruct
{
};

struct BC2 : DDSStruct
{
    // 16 4bit alphas
    uchar alpha[8];
};

struct BC3 : DDSStruct
{
    uchar alpha0;
    uchar alpha1;
    // 16 3bit alpha indices
    uchar alphaIndices[6];
};
/*
struct BC4 : DDSStruct
{

};

struct BC5 : DDSStruct
{

};
*/

} // namespace Common
} // namespace NFE