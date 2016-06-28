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

#if defined(WIN32)
struct FileAsync::AsyncDataStruct
{
    OVERLAPPED   overlapped;     //< OS-specific data
    DWORD        bytesToProcess; //< Parameter for Read/Write
    bool         isRead;         //< Operation type flag
    FileAsync*   instancePtr;    //< 'this' pointer
    void*        userData;       //< Pointer to user-defined object
    void*        dataBuffer;     //< Parameter for Read/Write
};
#elif defined(__LINUX__) | defined(__linux__)
struct FileAsync::AsyncDataStruct
{
    aiocb      aioData;     //< OS-specific data
    bool       isRead;      //< Operation type flag
    FileAsync* instancePtr; //< 'this' pointer
    void*      userData;    //< Pointer to user-defined object
};
#endif // defined(WIN32)

bool FileAsync::SafeErasePtr(AsyncDataStruct* ptrToErase)
{
    std::lock_guard<std::mutex> guard(mSetAccessMutex);
    bool eraseResult = 1 == mSystemPtrs.erase(ptrToErase);
    if (ptrToErase)
        delete ptrToErase;
    return  eraseResult;
}

bool FileAsync::SafeInsertPtr(AsyncDataStruct* ptrToInsert)
{
    std::lock_guard<std::mutex> guard(mSetAccessMutex);
    return mSystemPtrs.insert(ptrToInsert).second;
}

AccessMode FileAsync::GetFileMode() const
{
    return mMode;
}

} // namespace Common
} // namespace NFE
