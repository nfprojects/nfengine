/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Main source file for Performance tests
 */

#include "PCH.hpp"

#include "nfCommon/FileSystem/FileSystem.hpp"
#include "nfCommon/Reflection/ReflectionTypeRegistry.hpp"


int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    std::string execPath = NFE::Common::FileSystem::GetExecutablePath();
    std::string execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");
    NFE::Common::PathType logsDir = NFE::Common::FileSystem::GetPathType("Logs/PerfTests");
    if (logsDir != NFE::Common::PathType::Directory)
        NFE::Common::FileSystem::CreateDir("Logs/PerfTests");

    int result = RUN_ALL_TESTS();

    NFE::RTTI::TypeRegistry::GetInstance().Cleanup();

    // enable memory leak detection at the process exit (Windows only)
#ifdef _CRTDBG_MAP_ALLOC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _CRTDBG_MAP_ALLOC

    return result;
}
