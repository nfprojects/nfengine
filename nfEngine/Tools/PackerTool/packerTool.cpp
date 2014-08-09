#include "packerTool.hpp"

#include <memory>

#include "packerToolCallbacks.hpp"

using namespace std;

int main() try
{
    cout << "PackerTool\nCreated by LKostyra, 2013-2014.\nFor additional information type \"help\" command.\n\n";

    std::unique_ptr<packerToolUI> p_UI(new packerToolUI());

    if (p_UI.get())
    {
        AddCommands(p_UI.get());

        p_UI->MainLoop();
    }

    //old code, used only for reference for future Packer development
    /*Packer_CreateWriter(&p_VFSWriter);

    pr = p_VFSWriter->Init("PAKFILE.nfp");
    PACK_ERROR_COUT(pr);

    pr = p_VFSWriter->AddFilesRecursively("..\\Data");
    PACK_ERROR_COUT(pr);

    pr = p_VFSWriter->WritePAK();
    PACK_ERROR_COUT(pr);

    Packer_ReleaseWriter();

    Packer_CreateReader(&p_VFSReader);

    pr = p_VFSReader->Init("PAKFILE.nfp");
    PACK_ERROR_COUT(pr);

    cout << "Files in PAK archive: " << p_VFSReader->GetFileCount() << endl;

    Buffer ReadBuf;
    p_VFSReader->Read("Data\\config.cfg", ReadBuf);
    cout << "Read " << ReadBuf.GetSize() << " bytes of data.\n";

    /*p_VFSReader->Read("Data\\Fonts\\tahoma.ttf", 0, 0);
    p_VFSReader->Read("blabla", 0, 0);
    p_VFSReader->Read("Data\\Fonts", 0, 0);
    p_VFSReader->Read("Data\\Materials\\Material__298.cfg", 0, 0);

    Packer_ReleaseReader();

    system("PAUSE");*/

    p_UI.release();

#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}
catch (packerToolException& e)
{
    std::cout << "Packer exception caught at main: ";
    PrintColored(e.GetMsg(), ConsoleColor::RED | ConsoleColor::INTENSE);
    std::cout << "Aborting program execution." << std::endl;
    getchar();
}
catch (std::exception& e)
{
    std::cout << "STL exception caught at main: ";
    PrintColored(e.what(), ConsoleColor::RED | ConsoleColor::INTENSE);
    std::cout << "Aborting program execution." << std::endl;
    getchar();
}
catch (...)
{
    std::cout << "Unknown exception caught at main.\nAborting program execution." << std::endl;
    getchar();
}
