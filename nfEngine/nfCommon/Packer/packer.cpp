/**
    NFEngine project

    \file   packer.cpp
    \author LKostyra (costyrra.xl@gmail.com)
    \brief  Main module for Packer, which includes all other modules.
*/

#include "stdafx.hpp"
#include "packer.hpp"

namespace NFE {
namespace Common {

std::unique_ptr<PackWriter> Writer(nullptr);
std::unique_ptr<PackReader> Reader(nullptr);

PACK_RESULT Packer_CreateWriter(PackWriter** retwriter)
{
    if (retwriter == 0)
        return PACK_RESULT::NULLPOINTER;

    Writer.reset(new NFE::Common::PackWriter);
    *retwriter = Writer.get();

    return PACK_RESULT::OK;
}

PACK_RESULT Packer_CreateReader(PackReader** retreader)
{
    if (retreader == 0)
        return PACK_RESULT::NULLPOINTER;

    Reader.reset(new NFE::Common::PackReader);
    *retreader = Reader.get();

    return PACK_RESULT::OK;
}

void Packer_ReleaseWriter()
{
    Writer.reset(nullptr);
}

void Packer_ReleaseReader()
{
    Reader.reset(nullptr);
}

} // namespace Common
} // namespace NFE
