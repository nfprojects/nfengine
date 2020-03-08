/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Main source file for Performance tests
 */

#include "PCH.hpp"

#include "Engine/Common/FileSystem/FileSystem.hpp"
#include "Engine/Common/Containers/String.hpp"


using namespace NFE::Common;

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    const StringView logsDirPath("Logs/PerfTests");

    const String execPath = FileSystem::GetExecutablePath();
    const String execDir = FileSystem::GetParentDir(execPath);
    FileSystem::ChangeDirectory(execDir + "/../../..");
    PathType logsDir = FileSystem::GetPathType(logsDirPath);
    if (logsDir != PathType::Directory)
        FileSystem::CreateDir(logsDirPath);

    int result = RUN_ALL_TESTS();

    NFE::Common::ShutdownSubsystems();

    // enable memory leak detection at the process exit (Windows only)
#ifdef _CRTDBG_MAP_ALLOC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _CRTDBG_MAP_ALLOC

    return result;
}
