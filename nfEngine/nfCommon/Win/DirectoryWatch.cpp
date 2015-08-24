/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Windows implementation of DirectoryWatch class
 */

#include "../PCH.hpp"
#include "../DirectoryWatch.hpp"
#include "../Logger.hpp"
#include "Common.hpp"

namespace NFE {
namespace Common {

#define BUFFER_SIZE 16384

WatchRequest::WatchRequest()
{
    dirHandle = INVALID_HANDLE_VALUE;

    frontBuffer.resize(BUFFER_SIZE);
    backBuffer.resize(BUFFER_SIZE);
    
    ::ZeroMemory(&overlapped, sizeof(OVERLAPPED));
    overlapped.hEvent = static_cast<HANDLE>(this);
}

WatchRequest::~WatchRequest()
{
    if (dirHandle != INVALID_HANDLE_VALUE)
    {
        ::CancelIo(dirHandle);
        ::CloseHandle(dirHandle);
    }
}

bool WatchRequest::BeginRead()
{
    DWORD bytes = 0;
    BOOL success = ::ReadDirectoryChangesW(dirHandle,
                                           frontBuffer.data(), BUFFER_SIZE,
                                           TRUE,
                                           FILE_NOTIFY_CHANGE_LAST_WRITE |
                                           FILE_NOTIFY_CHANGE_CREATION |
                                           FILE_NOTIFY_CHANGE_FILE_NAME |
                                           FILE_NOTIFY_CHANGE_DIR_NAME |
                                           FILE_NOTIFY_CHANGE_LAST_ACCESS |
                                           FILE_NOTIFY_CHANGE_SIZE,
                                           &bytes, &overlapped,
                                           &DirectoryWatch::NotificationCompletion);
    if (success == 0)
    {
        LOG_ERROR("ReadDirectoryChangesW() failed: %s", GetLastErrorString().c_str());
        return false;
    }

    return true;
}


DirectoryWatch::DirectoryWatch()
{
    mRunning.store(true);

    mThread = ::CreateThread(0, 0, Dispatcher, this, 0, 0);
}

DirectoryWatch::~DirectoryWatch()
{
    mRunning.store(false);
    
    ::QueueUserAPC(&TerminateProc, mThread, reinterpret_cast<ULONG_PTR>(this));
    ::WaitForSingleObject(mThread, INFINITE);
}

void DirectoryWatch::SetCallback(WatchCallback callback)
{
    mCallback = callback;
}

bool DirectoryWatch::WatchPath(const std::string& path, Event eventFilter)
{
    (void)eventFilter;

    std::unique_ptr<WatchRequest> request(new WatchRequest);
    UTF8ToUTF16(path, request->widePath);
    request->watch = this;
    request->filter = static_cast<int>(eventFilter);

    // open directory
    request->dirHandle = ::CreateFile(
        request->widePath.c_str(),					            // pointer to the file name
        FILE_LIST_DIRECTORY,                                    // access (read/write) mode
        FILE_SHARE_READ	| FILE_SHARE_WRITE | FILE_SHARE_DELETE, // share mode
        NULL,                                                   // security descriptor
        OPEN_EXISTING,                                          // how to create
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,      // file attributes
        NULL);                                                  // file with attributes to copy

    if (request->dirHandle == INVALID_HANDLE_VALUE)
    {
        LOG_ERROR("CreateFile() failed for path '%s': %s", path.c_str(),
                  GetLastErrorString().c_str());
        return false;
    }

    ::QueueUserAPC(&AddDirectoryProc, mThread, reinterpret_cast<ULONG_PTR>(request.get()));

    {
        std::unique_lock<std::mutex> lock(mMutex);
        mDirs.insert(std::make_pair(path, std::move(request)));
        // TODO: trigger thread
    }

    LOG_DEBUG("Directory watch for path '%s' added", path.c_str());
    return true;
}

void DirectoryWatch::NotificationCompletion(DWORD errorCode, DWORD bytesTransferred,
                                            LPOVERLAPPED overlapped)
{
    WatchRequest* request = static_cast<WatchRequest*>(overlapped->hEvent);

    if (errorCode == ERROR_OPERATION_ABORTED)
    {
        // TODO: remove watch request
        return;
    }

    if (bytesTransferred == 0)
        return;

    assert(bytesTransferred <= request->frontBuffer.size());
    memcpy(request->backBuffer.data(), request->frontBuffer.data(), bytesTransferred);

    // start receiving new events as fast as possible
    request->BeginRead();

    std::string shortPath;
    char* base = request->backBuffer.data();
    for (;;)
    {
        FILE_NOTIFY_INFORMATION& fni = (FILE_NOTIFY_INFORMATION&)*base;
        DirectoryWatch::Event eventType;
        DirectoryWatch::Event eventFiler = static_cast<DirectoryWatch::Event>(request->filter);

        switch (fni.Action)
        {
        case FILE_ACTION_ADDED:
            eventType = Event::Create;
            break;
        case FILE_ACTION_REMOVED:
            eventType = Event::Delete;
            break;
        case FILE_ACTION_MODIFIED:
            eventType = Event::Modify;
            break;
        case FILE_ACTION_RENAMED_OLD_NAME:
            eventType = Event::MoveFrom;
            break;
        case FILE_ACTION_RENAMED_NEW_NAME:
            eventType = Event::MoveTo;
            break;
        default:
            eventType = Event::None;
        };

        if ((eventFiler & eventType) == eventType)
        {
            std::wstring fileName(fni.FileName, fni.FileNameLength / sizeof(wchar_t));
            fileName = request->widePath + L'/' + fileName;
            UTF16ToUTF8(fileName, shortPath);

            DirectoryWatch::EventData data;
            data.type = eventType;
            data.path = shortPath.c_str();
            LOG_DEBUG("DirectoryWatch: action=%d, path=%s", fni.Action, shortPath.c_str());

            if (request->watch->mCallback)
                request->watch->mCallback(data);
        }

        if (!fni.NextEntryOffset)
            break;
        base += fni.NextEntryOffset;
    };
}

void DirectoryWatch::AddDirectoryProc(ULONG_PTR arg)
{
    WatchRequest* request = reinterpret_cast<WatchRequest*>(arg);
    request->BeginRead();
}

void DirectoryWatch::TerminateProc(ULONG_PTR arg)
{
    DirectoryWatch* watch = reinterpret_cast<DirectoryWatch*>(arg);

    std::unique_lock<std::mutex> lock(watch->mMutex);
    watch->mDirs.clear();
}

DWORD DirectoryWatch::Dispatcher(LPVOID param)
{
    DirectoryWatch* watch = static_cast<DirectoryWatch*>(param);

    while (watch->mRunning)
    {
        ::SleepEx(INFINITE, true);
    }

    return 0;
}

void DirectoryWatch::WatchRoutine()
{
}

} // namespace Common
} // namespace NFE
