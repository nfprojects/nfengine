/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Component's base definition.
 */

#include "PCH.hpp"
#include "Component.hpp"

namespace NFE {
namespace Scene {

std::vector<ComponentInfo> Component::mComponents;
int Component::mComponentIdCounter = 0;

void Component::Invalidate()
{
}

bool Component::Register(int id, const char* name, size_t size)
{
    ComponentInfo info;
    info.id = id;
    info.name = name;
    info.size = size;
    mComponents.push_back(info);

    return true;
}

} // namespace Scene
} // namespace NFE
