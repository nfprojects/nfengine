#include "stdafx.hpp"

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
    testing::InitGoogleTest(&argc, argv);

    SetUpCurrentDirectory();
    int result = RUN_ALL_TESTS();

    _CrtDumpMemoryLeaks();
    return result;
}
