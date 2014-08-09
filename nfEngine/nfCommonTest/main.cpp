#include "stdafx.h"

// temporary
void SetUpCurrentDirectory()
{
    char szFileName[MAX_PATH];
    GetModuleFileNameA(NULL, szFileName, MAX_PATH);
    std::string rootDir = std::string(szFileName);
    size_t found = rootDir.find_last_of("/\\");
    rootDir = rootDir.substr(0, found) + "/../../../";
    SetCurrentDirectoryA(rootDir.c_str());
}

int main(int argc, char* argv[])
{
    //PROFILE_COMMAND_STATUS profstat = StopProfile(PROFILE_GLOBALLEVEL, PROFILE_CURRENTID);
    testing::InitGoogleTest(&argc, argv);

    SetUpCurrentDirectory();
    int result = RUN_ALL_TESTS();

    _CrtDumpMemoryLeaks();
    system("pause");
    return result;
}
