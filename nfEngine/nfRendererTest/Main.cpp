#include "PCH.hpp"
#include "Backends.hpp"
#include "../nfCommon/FileSystem.hpp"

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    std::string execPath = NFE::Common::FileSystem::GetExecutablePath();
    std::string execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");

    // select renderer and set global variable for it
    std::string rend;
    if (argc < 2)
    {
        std::vector<std::string> defBackend = GetDefaultBackend();
        rend = defBackend[0];
        gShaderPathPrefix = defBackend[1];
        gShaderPathExt = defBackend[2];
    }
    else if (D3D11_BACKEND.compare(argv[1]) == 0)
    {
        // we use D3D11 renderer
        rend = D3D11_BACKEND;
        gShaderPathPrefix = D3D11_SHADER_PATH_PREFIX;
        gShaderPathExt = D3D11_SHADER_EXTENSION;
    }
    else if (OGL4_BACKEND.compare(argv[1]) == 0)
    {
        rend = OGL4_BACKEND;
        gShaderPathPrefix = OGL4_SHADER_PATH_PREFIX;
        gShaderPathExt = OGL4_SHADER_EXTENSION;
    }
    else
    {
        std::cerr << "Incorrect backend provided" << std::endl;
        return 1;
    }

    return RUN_ALL_TESTS();
}
