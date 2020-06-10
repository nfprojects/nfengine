/**
 * @file
 */

#include "PCH.hpp"
#include "nfCommon.hpp"
#include "Reflection/ReflectionTypeRegistry.hpp"
#include "Logger/LoggerImpl.hpp"


namespace NFE {
namespace Common {


bool InitSubsystems()
{
    if (!Logger::GetInstance()->Init())
        return false;

    return true;
}

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
