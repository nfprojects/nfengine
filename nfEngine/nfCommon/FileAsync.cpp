/**
 * @FileAsync
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Platform independent methods implementation from FileAsync class.
 */

#include "PCH.hpp"
#include "FileAsync.hpp"
#include "nfCommon.hpp"


namespace NFE {
namespace Common {
struct FileAsync::AsyncDataStruct
{
    OVERLAPPED   overlapped;  //< OS-specific data
    bool         isRead;      //< Operation type flag
    FileAsync*   instancePtr; //< 'this' pointer
    void*        userData;    //< Pointer to user-defined object
};


bool FileAsync::SafeErasePtr(AsyncDataStruct* ptrToErase)
{
    std::lock_guard<std::mutex> guard(mSetAccessMutex);
    bool eraseResult = 1 == mSystemPtrs.erase(ptrToErase);
    delete ptrToErase;
    return  eraseResult;
}

bool FileAsync::SafeInsertPtr(AsyncDataStruct* ptrToInsert)
{
    std::lock_guard<std::mutex> guard(mSetAccessMutex);
    return mSystemPtrs.insert(ptrToInsert).second;
}

} // namespace Common
} // namespace NFE
