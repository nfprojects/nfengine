#include "stdafx.hpp"
#include "packerToolCallbacks.hpp"

#include <memory>
#include <functional>
#include <limits>
#include <istream>

#include <Packer/packer.hpp>
using namespace NFE::Common;

std::string s_FilePath;
const std::string cs_Extension = ".pak";
bool b_isReading = false;
std::unique_ptr<PackerWriter> VFSWriter;
std::unique_ptr<PackerReader> VFSReader;

/////////////////
//HELP MESSAGES//
/////////////////
const char* helpStr_load = R"(Loads PAK archive into memory.
      Syntax:
            load <file_path>

      Parameters:
      + file_path - path to PAK archive
)";

const std::string helpStr_create = R"(Initializes new PAK archive.\n\
      Syntax:
            create <file_path>

      Parameters:
      + file_path - path where new PAK archive will be created
)";

const std::string helpStr_add = R"(Adds a file to PAK Archive.
      Syntax:
        add <file_path> <vfs_file_path>

      Parameters:
      + file_path - path to file to be added
      + vfs_file_path - path which will be used when loading a file from PAK

Keep in mind - PAK must be saved using SAVE command for changes to apply!
)";

const std::string helpStr_save = R"(
Saves PAK archive to path specified during "create" call.
      Syntax:
        save

      Parameters:
        This function takes no additional arguments.
)";

////////////////////////////////
//NEXT COMMAND TOKEN EXTRACTOR//
////////////////////////////////
std::string GetNextToken(std::string& cmd)
{
    std::string ret, newcmd;

    if (cmd.empty())
        return newcmd;

    std::string::iterator it = cmd.begin();

    //check if next word is a quotation mark
    if (*it == '\"')
    {
        //skip quotation
        it++;

        //copy entire token until you meet another quotation mark
        while (it != cmd.end() && *it != '\"')
            ret += *it++;

        //skip second quotation
        it++;
    }
    else
    {
        //copy next word to external string
        while (it != cmd.end() && *it != ' ' && *it != '\n')
            ret += *it++;
    }

    //skip all spaces from input until next word appears (if any exists)
    while (it != cmd.end() && *it == ' ')
        it++;

    //replace cmd contents with rest of string
    while (it != cmd.end())
        newcmd += *it++;

    std::cout << newcmd << std::endl;

    cmd = newcmd;

    return ret;
}

//////////////////
//INIT CALLBACKS//
//////////////////
void OnBeforeLoop()
{
    ///TODO Add error code translated to string
    PackerResult pr = PackerResult::OK;

    VFSWriter.reset(new PackerWriter);
    AssertMsg(pr == PackerResult::OK, "Error: " + PACK_RESULT_TO_STRING(pr));

    VFSReader.reset(new PackerReader);
    AssertMsg(pr == PackerResult::OK, "Error: " + PACK_RESULT_TO_STRING(pr));
}

void OnAfterLoop()
{
    VFSWriter.reset();
    VFSReader.reset();
}

///////////////////
//PACKER COMMANDS//
///////////////////
void Callback_LoadArchive(std::string& cmdString)
{
    std::string cmd = GetNextToken(cmdString);

    if (cmd.empty())
    {
        std::cout << "Missing argument! Check help for correct syntax!" << std::endl;
        return;
    }

    if (cmd == "help")
    {
        std::cout << helpStr_load;
        return;
    }

    PackerResult pr = VFSReader->Init(cmd);
    AssertMsg(pr == PackerResult::OK, "Failed to load Pack Archive. Packer DLL result: " +
              PACK_RESULT_TO_STRING(pr) + " - " + Packer_GetErrorStr(pr));

    b_isReading = true;
    std::cout << "Archive loaded to Reader.\n";
}

void Callback_CreateArchive(std::string& cmdString)
{
    std::string cmd = GetNextToken(cmdString);

    if (cmd.empty())
    {
        std::cout << "Missing argument! Check help for correct syntax!" << std::endl;
        return;
    }

    if (cmd == "help")
    {
        std::cout << helpStr_create;
        return;
    }

    //add .pak to the end of archive name if it does not exist
    if (cmd.size() < 4 || cmd.rfind(cs_Extension) == std::string::npos)
        cmd += cs_Extension;

    FILE* file = fopen(cmd.c_str(), "r");
    if (file != NULL)
    {
        // TODO: use dedicated function (when it's created) to check if a file exists
        fclose(file);

        char c;
        std::cout << "PAK under given name already exists! Overwrite? (y/n): ";
        std::cin >> c;

        if (c == 'y')
            std::cout <<
                      "Creating PAK. Keep in mind, archive will be overwritten only after \"save\" command.\n";
        else
        {
            std::cout << "File won't be overwritten. Aborting creation of new PAK.\n";
            return;
        }

        //ignore anything waiting in the stream
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    PackerResult pr = VFSWriter->Init(cmd);
    AssertMsg(pr == PackerResult::OK, "Failed to initialize new Pack Archive. Packer DLL result: " +
              PACK_RESULT_TO_STRING(pr) + " - " + Packer_GetErrorStr(pr));

    s_FilePath = cmd;

    b_isReading = false;
}

void Callback_AddFile(std::string& cmdString)
{
    std::string path = GetNextToken(cmdString);

    if (path.empty())
    {
        std::cout << "Missing argument! Check help for correct syntax!" << std::endl;
        return;
    }

    if (path == "help")
    {
        std::cout << helpStr_create;
        return;
    }

    std::string vfsPath = GetNextToken(cmdString);
    if (vfsPath.empty())
        vfsPath = path;

    PackerResult pr = VFSWriter->AddFile(path, vfsPath);
    AssertMsg(pr == PackerResult::OK, "Failed to add file to Pack Archive. Packer DLL result: " +
              PACK_RESULT_TO_STRING(pr) + " - " + Packer_GetErrorStr(pr));
}

void Callback_SaveArchive(std::string& cmdString)
{
    if (!cmdString.empty() && cmdString == "help")
    {
        std::cout << helpStr_save;
        return;
    }

    PackerResult pr = VFSWriter->WritePAK();
    AssertMsg(pr == PackerResult::OK, "Failed to save Pack Archive. Packer DLL result: " +
              PACK_RESULT_TO_STRING(pr) + " - " + Packer_GetErrorStr(pr));
}

void Callback_ListArchive(std::string&)
{
    if (b_isReading)
        VFSReader->PrintFilesToStdout();
    else
        VFSWriter->PrintFilesToStdout();
}

#ifdef _DEBUG
////////////////////////
//ENV TESTING COMMANDS//
////////////////////////
void Test_BasicCommand(std::string& cmdString)
{
    std::cout << "I am called back by Packer!!! yay!\n";

    if (cmdString.empty())
        std::cout << "There is no rest of command!\n";
    else
        std::cout << "Rest of command sent to me: " << cmdString << std::endl;
}

void Test_GetNextToken(std::string&)
{
    std::cout << "Checking getNextToken function...\n\n";

    std::string sampleCmd = "blabla test_command checking if \"everything works fine.\" Here too";

    std::cout << "Sample string:\n" << sampleCmd << "\n\n";

    std::vector<std::string> resTab;
    //divide tokens
    while (!sampleCmd.empty())
        resTab.push_back(GetNextToken(sampleCmd));

    std::cout << "Token division complete. Acquired " << resTab.size() << " tokens. Results:\n";

    unsigned int count = 0;
    std::for_each(resTab.begin(), resTab.end(), [&count](std::string el)
    {
        std::cout << "Token #" << count++ << ": " << el << "\n";
    });
}

void Test_ExceptionCatch(std::string& cmdString)
{
    std::string cmd = GetNextToken(cmdString);

    if (cmd == "packer")
        ThrowMsg("Packer exception thrown.");
    else if (cmd == "std")
        throw std::exception("STD exception thrown.");
    else if (cmd == "unknown")
        throw int(3);
    else
        std::cout << "Incorrect exception type.\nAvailable types:\n\tpacker\n\tstd\n\tunknown" << std::endl;
}
#endif

//Externally visible function supplying UI with commands
void AddCommands(packerToolUI* p_UI)
{
    Assert(p_UI != nullptr);

    p_UI->AddBeforeLoopCommand(&OnBeforeLoop);
    p_UI->AddAfterLoopCommand(&OnAfterLoop);

#ifdef _DEBUG
    //test commands
    p_UI->AddCommand("test_basic", &Test_BasicCommand,
                     "TEST: Used to test if command system works and if command is passed on successfully.");
    p_UI->AddCommand("test_token", &Test_GetNextToken,
                     "TEST: Used to test function responsible for splitting command into tokens.");
    p_UI->AddCommand("test_exception", &Test_ExceptionCatch,
                     "TEST: Used to test if exceptions are properly caught by callback caller.");
#endif

    //callbacks
    p_UI->AddCommand("load", &Callback_LoadArchive, "Load archive for further use.");
    p_UI->AddCommand("create", &Callback_CreateArchive, "Initialize new archive.");
    p_UI->AddCommand("add", &Callback_AddFile, "Adds file to current archive.");
    p_UI->AddCommand("save", &Callback_SaveArchive, "Saves current archive.");
    p_UI->AddCommand("list", &Callback_ListArchive, "Lists currently loaded archive.");
}
