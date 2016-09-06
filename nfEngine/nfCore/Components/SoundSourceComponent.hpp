/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#pragma once

#include "../Core.hpp"
#include "Component.hpp"
#include "nfCommon/Aligned.hpp"

namespace NFE {
namespace Scene {

NFE_ALIGN16
class CORE_API SoundSourceComponent
    : public ComponentBase<SoundSourceComponent>
{
    // TODO

public:
    SoundSourceComponent();
    virtual ~SoundSourceComponent();
};

} // namespace Scene
} // namespace NFE
