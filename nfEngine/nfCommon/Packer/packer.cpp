/**
    NFEngine project

    \file   packer.cpp
    \author LKostyra (costyrra.xl@gmail.com)
    \brief  Main module for Packer, which includes all other modules.
*/

#include "stdafx.h"
#include "packer.hpp"

namespace NFE {
namespace Common {

PackWriter* Writer = 0;
PackReader* Reader = 0;

PACK_RESULT Packer_CreateWriter(NFE::Common::PackWriter** retwriter)
{
    if (retwriter == 0)
        return PACK_RESULT::NULLPOINTER;

    if (Writer)
        delete Writer;

    Writer = new NFE::Common::PackWriter;
    *retwriter = Writer;

    return PACK_RESULT::OK;
}

PACK_RESULT Packer_CreateReader(NFE::Common::PackReader** retreader)
{
    if (retreader == 0)
        return PACK_RESULT::NULLPOINTER;

    if (Reader)
        delete Reader;

    Reader = new NFE::Common::PackReader;
    *retreader = Reader;

    return PACK_RESULT::OK;
}

void Packer_ReleaseWriter()
{
    delete Writer;
    Writer = 0;
}

void Packer_ReleaseReader()
{
    delete Reader;
    Reader = 0;
}

} // namespace Common
} // namespace NFE
