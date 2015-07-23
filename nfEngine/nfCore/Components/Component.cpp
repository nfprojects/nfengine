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

// implement empty destructor to avoid unresolved symbols
Component::~Component()
{
}

} // namespace Scene
} // namespace NFE
