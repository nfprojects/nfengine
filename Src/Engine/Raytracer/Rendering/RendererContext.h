#pragma once

#include "../Raytracer.h"
#include "../../Common/Containers/UniquePtr.hpp"

#include <memory>

namespace NFE {
namespace RT {

// per-thread renderer-specific context
class IRendererContext
{
    NFE_MAKE_NONCOPYABLE(IRendererContext);
    NFE_MAKE_NONMOVEABLE(IRendererContext);

public:
    IRendererContext();
    virtual ~IRendererContext();
};

using RendererContextPtr = Common::UniquePtr<IRendererContext>;

} // namespace RT
} // namespace NFE
