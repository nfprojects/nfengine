/**
 * @FileAsync
 * @author mkulagowski (mkkulagowski(at)gmail.com)
 * @brief  Platform independent methods implementation from FileAsync class.
 */

#include "PCH.hpp"
#include "FileAsync.hpp"
#include "../Utils/ScopedLock.hpp"


namespace NFE {
namespace Common {

#if defined(NFE_PLATFORM_WINDOWS)
struct FileAsync::AsyncDataStruct
{
    OVERLAPPED   overlapped;     //< OS-specific data
    DWORD        bytesToProcess; //< Parameter for Read/Write
    bool         isRead;         //< Operation type flag
    FileAsync*   instancePtr;    //< 'this' pointer
    void*        userData;       //< Pointer to user-defined object
    void*        dataBuffer;     //< Parameter for Read/Write
};
#elif defined(NFE_PLATFORM_LINUX)
struct FileAsync::AsyncDataStruct
{
    iocb       ioData;      //< OS-specific data
    bool       isRead;      //< Operation type flag
    FileAsync* instancePtr; //< 'this' pointer
    void*      userData;    //< Pointer to user-defined object
};
#else
#error Invalid platform
#endif

bool FileAsync::SafeErasePtr(AsyncDataStruct* ptrToErase)
{
    NFE_SCOPED_LOCK(mSetAccessMutex);
    bool eraseResult = mSystemPtrs.Erase(ptrToErase);
    if (ptrToErase)
        delete ptrToErase;
    return  eraseResult;
}

bool FileAsync::SafeInsertPtr(AsyncDataStruct* ptrToInsert)
{
    NFE_SCOPED_LOCK(mSetAccessMutex);
    return mSystemPtrs.Insert(ptrToInsert).iterator != mSystemPtrs.End();
}

AccessMode FileAsync::GetFileMode() const
{
    return mMode;
}

} // namespace Common
} // namespace NFE
