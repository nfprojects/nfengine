/**
 * @file
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Thread class definition.
 */

#include "../PCH.hpp"
#include "../Thread.hpp"
#include <thread>


namespace NFE {
namespace Common {

bool Thread::SetThreadPriority(std::thread& thread, ThreadPriority priority)
{
    auto result = ::SetThreadPriority(thread.native_handle(), static_cast<int>(priority));
    return (result!= 0);
}

} // namespace Common
} // namespace NFE
