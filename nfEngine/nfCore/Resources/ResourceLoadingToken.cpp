/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 */

#include "PCH.hpp"
#include "ResourceLoadingToken.hpp"

#include "nfCommon/System/Memory.hpp"
#include "nfCommon/Logger/Logger.hpp"
#include "nfCommon/Utils/AsyncThreadPool.hpp"


namespace NFE {
namespace Resource {

LoadingToken::LoadingToken(Common::TaskID loadingTask)
    : mLoadingTask(loadingTask)
{
}

LoadingToken::LoadingToken(const ResourceHandle& resource)
    : mLoadedResource(resource)
{
}

LoadingToken::~LoadingToken()
{
    // TODO cancel resource loading
}

ResourceHandle LoadingToken::WaitForResource()
{

}

} // namespace Resource
} // namespace NFE
