/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Main source file for Performance tests
 */

#include "PCH.hpp"
#include "nfCommon/FileSystem/FileSystem.hpp"

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    std::string execPath = NFE::Common::FileSystem::GetExecutablePath();
    std::string execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");
    NFE::Common::PathType logsDir = NFE::Common::FileSystem::GetPathType("Logs/PerfTests");
    if (logsDir != NFE::Common::PathType::Directory)
        NFE::Common::FileSystem::CreateDir("Logs/PerfTests");

    return RUN_ALL_TESTS();
}
