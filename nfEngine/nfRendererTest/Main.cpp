#include "PCH.hpp"
#include "Backends.hpp"
#include "nfCommon/FileSystem.hpp"

const std::string BACKEND_ARG_NAME = "--renderer";

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);

    std::string execPath = NFE::Common::FileSystem::GetExecutablePath();
    std::string execDir = NFE::Common::FileSystem::GetParentDir(execPath);
    NFE::Common::FileSystem::ChangeDirectory(execDir + "/../../..");

    // find a backend argument - GTest arguments might confuse our simple arg check
    // which was used in nfRendererDemo
    std::string selectedBackend;
    for (int i = 0; i < argc; ++i)
    {
        if ((BACKEND_ARG_NAME.compare(argv[i]) == 0) && (i + 1 < argc))
        {
            selectedBackend = argv[i+1];
            break;
        }
    }

    // select renderer and set global variable for it
    if (selectedBackend.empty())
    {
        // no backend argument, use default
        std::vector<std::string> defBackend = GetDefaultBackend();
        gBackend = defBackend[0];
        gShaderPathPrefix = defBackend[1];
        gShaderPathExt = defBackend[2];
    }
    else if (D3D11_BACKEND.compare(selectedBackend) == 0)
    {
        // we use D3D11 renderer
        gBackend = D3D11_BACKEND;
        gShaderPathPrefix = D3D11_SHADER_PATH_PREFIX;
        gShaderPathExt = D3D11_SHADER_EXTENSION;
    }
    else if (OGL4_BACKEND.compare(selectedBackend) == 0)
    {
        // we use OGL4 renderer
        gBackend = OGL4_BACKEND;
        gShaderPathPrefix = OGL4_SHADER_PATH_PREFIX;
        gShaderPathExt = OGL4_SHADER_EXTENSION;
    }
    else
    {
        // we want to use something else, which probably isn't supported yet
        std::cerr << "Incorrect backend provided" << std::endl;
        return 1;
    }

    int ret = RUN_ALL_TESTS();

    // enable memory leak detection at the process exit (Windows only)
#ifdef _CRTDBG_MAP_ALLOC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _CRTDBG_MAP_ALLOC

    return ret;
}
