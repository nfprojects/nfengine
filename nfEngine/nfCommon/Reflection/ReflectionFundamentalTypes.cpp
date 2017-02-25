/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Definitions of fundamental types for the reflection system.
 */

#include "PCH.hpp"
#include "../nfCommon.hpp"
#include "ReflectionFundamentalTypes.hpp"
#include "ReflectionTypeSerialization.hpp"
#include "../Config/Config.hpp"



NFE_DEFINE_FUNDAMENTAL_TYPE(bool);
NFE_DEFINE_FUNDAMENTAL_TYPE(float);
NFE_DEFINE_FUNDAMENTAL_TYPE(double);

NFE_DEFINE_FUNDAMENTAL_TYPE_NAMESPACE(NFE, uint8);
NFE_DEFINE_FUNDAMENTAL_TYPE_NAMESPACE(NFE, uint16);
NFE_DEFINE_FUNDAMENTAL_TYPE_NAMESPACE(NFE, uint32);
NFE_DEFINE_FUNDAMENTAL_TYPE_NAMESPACE(NFE, uint64);

NFE_DEFINE_FUNDAMENTAL_TYPE_NAMESPACE(NFE, int8);
NFE_DEFINE_FUNDAMENTAL_TYPE_NAMESPACE(NFE, int16);
NFE_DEFINE_FUNDAMENTAL_TYPE_NAMESPACE(NFE, int32);
NFE_DEFINE_FUNDAMENTAL_TYPE_NAMESPACE(NFE, int64);