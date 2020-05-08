#pragma once

#include "../nfCommon.hpp"
#include "Object.hpp"
#include "../Containers/DynArray.hpp"
#include "../Containers/SharedPtr.hpp"


namespace NFE {
namespace RTTI {

// serialize single object into binary data stream
NFCOMMON_API bool Serialize(const ObjectPtr& rootObject, Common::OutputStream& outputStream);

// serialize multiple objects into binary data stream
NFCOMMON_API bool Serialize(const Common::ArrayView<const ObjectPtr> rootObjects, Common::OutputStream& outputStream);

// deserialize multiple objects from a binary data stream
NFCOMMON_API bool Deserialize(Common::DynArray<ObjectPtr>& outRootObjects, Common::InputStream& inputStream);

} // namespace RTTI
} // namespace NFE
