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

/////////////////
//HELP MESSAGES//
/////////////////
const std::string helpStr_load = "\
Loads PAK archive into memory.\n\
      Syntax:\n\
            load <file_path>\n\
\n\
      Parameters:\n\
      + file_path - path to PAK archive\
";

const std::string helpStr_create = "\
Initializes new PAK archive.\n\
      Syntax:\n\
            create <file_path>\n\
\n\
      Parameters:\n\
      + file_path - path where new PAK archive will be created\
";

const std::string helpStr_add = "\
Adds a file to PAK Archive.\n\
      Syntax:\n\
        add <file_path> <vfs_file_path>\n\
\n\
      Parameters:\n\
      + file_path - path to file to be added\n\
      + vfs_file_path - path which will be used when loading a file from PAK\n\
\n\
Keep in mind - PAK must be saved using SAVE command for changes to apply!\
";

const std::string helpStr_save = "\
Saves PAK archive to path specified during \"create\" call.\n\
      Syntax:\n\
        save\n\
\n\
      Parameters:\n\
        This function takes no additional arguments.\
";

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

///////////////////////
//UNIQUE PTR TYPEDEFS//
///////////////////////
void WriterDeleter(NFE::Common::PackWriter* ptr)
{
    Packer_ReleaseWriter();
    ptr = nullptr;
}
typedef std::unique_ptr<PackWriter, std::function<void(PackWriter*)>> WriterUniquePtr;

void ReaderDeleter(NFE::Common::PackReader* ptr)
{
    Packer_ReleaseReader();
    ptr = nullptr;
}
typedef std::unique_ptr<PackReader, std::function<void(PackReader*)>> ReaderUniquePtr;

WriterUniquePtr p_VFSWriter(nullptr, WriterDeleter);
ReaderUniquePtr p_VFSReader(nullptr, ReaderDeleter);

//////////////////
//INIT CALLBACKS//
//////////////////
void OnBeforeLoop()
{
    ///TODO Add error code translated to string
    PACK_RESULT pr = PACK_RESULT::OK;

    PackWriter* tmpWriter;
    pr = Packer_CreateWriter(&tmpWriter);
    AssertMsg(pr == PACK_RESULT::OK, "Error: " + PACK_RESULT_TO_STRING(pr));

    p_VFSWriter.reset(tmpWriter);

    PackReader* tmpReader;
    pr = Packer_CreateReader(&tmpReader);
    AssertMsg(pr == PACK_RESULT::OK, "Error: " + PACK_RESULT_TO_STRING(pr));

    p_VFSReader.reset(tmpReader);
}

void OnAfterLoop()
{
    p_VFSReader.release();
    p_VFSWriter.release();
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

    PACK_RESULT pr = p_VFSReader->Init(cmd);
    AssertMsg(pr == PACK_RESULT::OK, "Failed to load Pack Archive. Packer DLL result: " +
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

    FILE* p_File = fopen(cmd.c_str(), "r");
    if (p_File != NULL)
    {
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

    PACK_RESULT pr = p_VFSWriter->Init(cmd);
    AssertMsg(pr == PACK_RESULT::OK, "Failed to initialize new Pack Archive. Packer DLL result: " +
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

    PACK_RESULT pr = p_VFSWriter->AddFile(path, vfsPath);
    AssertMsg(pr == PACK_RESULT::OK, "Failed to add file to Pack Archive. Packer DLL result: " +
              PACK_RESULT_TO_STRING(pr) + " - " + Packer_GetErrorStr(pr));
}

void Callback_SaveArchive(std::string& cmdString)
{
    if (!cmdString.empty() && cmdString == "help")
    {
        std::cout << helpStr_save;
        return;
    }

    PACK_RESULT pr = p_VFSWriter->WritePAK();
    AssertMsg(pr == PACK_RESULT::OK, "Failed to save Pack Archive. Packer DLL result: " +
              PACK_RESULT_TO_STRING(pr) + " - " + Packer_GetErrorStr(pr));
}

void Callback_ListArchive(std::string&)
{
    if (b_isReading)
        p_VFSReader->PrintFilesToStdout();
    else
        p_VFSWriter->PrintFilesToStdout();
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
