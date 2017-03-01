#include "PCH.hpp"
#include "packerTool.hpp"

#include <memory>

#include "packerToolCallbacks.hpp"

int main()
{
    std::cout <<
              "PackerTool\nCreated by LKostyra, 2013-2014.\nFor additional information type \"help\" command.\n\n";

    std::unique_ptr<packerToolUI> p_UI(new packerToolUI());

    if (p_UI.get())
    {
        AddCommands(p_UI.get());

        p_UI->MainLoop();
    }

    p_UI.release();

#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}
