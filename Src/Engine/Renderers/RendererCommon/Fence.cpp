/**
 * @file
 * @author  Witek902
 */

#include "PCH.hpp"
#include "Fence.hpp"
#include "Engine/Common/Utils/TaskBuilder.hpp"
#include "Engine/Common/Utils/Waitable.hpp"


namespace NFE {
namespace Renderer {

using namespace Common;

IFence::~IFence() = default;

void IFence::Wait()
{
    Waitable waitable;
    {
        TaskBuilder builder(waitable);
        Sync(builder);
    }
    waitable.Wait();
}

} // namespace Renderer
} // namespace NFE
