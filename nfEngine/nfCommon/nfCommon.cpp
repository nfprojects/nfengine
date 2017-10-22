/**
 * @file
 */

#include "PCH.hpp"
#include "nfCommon.hpp"
#include "Reflection/ReflectionTypeRegistry.hpp"
#include "Logger/Logger.hpp"


namespace NFE {
namespace Common {


void ShutdownSubsystems()
{
    // cleanup registered RTTI types
    RTTI::TypeRegistry::GetInstance().Cleanup();

    // shutdown logger
    Logger::GetInstance()->Shutdown();

    // shutdown default memory allocator
    // DefaultAllocator::GetInstance().Shutdown();
}


} // namespace Common
} // namespace NFE
