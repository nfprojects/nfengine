/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base class declarations.
 */

#pragma once

#include "../Core.hpp"
#include "../Aligned.hpp"

namespace NFE {
namespace Scene {

/**
 * Base abstract class for entity component.
 */
NFE_ALIGN16
class CORE_API Component : public Util::Aligned
{
};

} // namespace Scene
} // namespace NFE
