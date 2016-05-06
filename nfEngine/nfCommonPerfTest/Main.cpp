/**
 * @file
 * @author LKostyra (costyrra.xl@gmail.com)
 * @brief  Main source file for Performance tests
 */

#include "PCH.hpp"
#include "FileSystem.hpp"

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    std::string execPath = NFE::Common::FileSystem::GetExecutablePath();
    std::string execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../../Logs");
    NFE::Common::FileSystem::CreateDir("PerfTests");
    NFE::Common::FileSystem::ChangeDirectory("PerfTests");

    return RUN_ALL_TESTS();
}
