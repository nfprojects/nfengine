/**
    NFEngine project

    \file   packer.hpp
    \author LKostyra (costyrra.xl@gmail.com)
    \brief  Main module for Packer, which includes all other modules.
*/

#pragma once

#include "packerDefines.hpp"

#ifdef NFCOMMON_EXPORTS
#include "packerElement.hpp"
#endif

#include "packerReader.hpp"
#include "packerWriter.hpp"

namespace NFE {
namespace Common {

NFCOMMON_API PACK_RESULT Packer_CreateWriter(PackWriter** retwriter);
NFCOMMON_API PACK_RESULT Packer_CreateReader(PackReader** retreader);
NFCOMMON_API void Packer_ReleaseWriter();
NFCOMMON_API void Packer_ReleaseReader();

} // namespace Common
} // namespace NFE
