/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base definition.
 */

#include "../PCH.hpp"
#include "Component.hpp"

namespace NFE {
namespace Scene {

int Component::gComponentIdCounter = 0;

void Component::Invalidate()
{
}

} // namespace Scene
} // namespace NFE
