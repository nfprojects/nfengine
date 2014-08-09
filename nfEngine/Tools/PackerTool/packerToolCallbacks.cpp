#include "packerToolCallbacks.hpp"

#include <memory>
#include <functional>

// TODO uncomment when Packer becomes rewritten
//#include <libpacker\libpacker.h>

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
// TODO Uncomment when Packer is rewritten
/*
void WriterDeleter(XPackWriter* ptr)
{
    Packer_ReleaseWriter();
    ptr = nullptr;
}
typedef std::unique_ptr<XPackWriter, std::function<void(XPackWriter*)>> WriterUniquePtr;

void ReaderDeleter(XPackReader* ptr)
{
    Packer_ReleaseReader();
    ptr = nullptr;
}
typedef std::unique_ptr<XPackReader, std::function<void(XPackReader*)>> ReaderUniquePtr;

WriterUniquePtr p_VFSWriter(nullptr, WriterDeleter);
ReaderUniquePtr p_VFSReader(nullptr, ReaderDeleter);
*/
//////////////////
//INIT CALLBACKS//
//////////////////
void OnBeforeLoop()
{
    ///TODO Add error code translated to string
    /*PACK_RESULT pr = PACK_OK;

    XPackWriter* tmpWriter;
    pr = Packer_CreateWriter(&tmpWriter);
    AssertMsg(pr == PACK_OK, "Error: " + std::to_string(pr));

    p_VFSWriter.reset(tmpWriter);

    XPackReader* tmpReader;
    pr = Packer_CreateReader(&tmpReader);
    AssertMsg(pr == PACK_OK, "Error: " + std::to_string(pr));

    p_VFSReader.reset(tmpReader);*/
}

void OnAfterLoop()
{
    /*p_VFSReader.release();
    p_VFSWriter.release();*/
}

///////////////////
//PACKER COMMANDS//
///////////////////
//TODO implement those commands as a part of code refactoring

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


//Externally visible function supplying UI with commands
void AddCommands(packerToolUI* p_UI)
{
    Assert(p_UI != nullptr);

    p_UI->AddBeforeLoopCommand(&OnBeforeLoop);
    p_UI->AddAfterLoopCommand(&OnAfterLoop);

    //test commands
    p_UI->AddCommand("test_basic", &Test_BasicCommand,
                     "TEST: Used to test if command system works and if command is passed on successfully.");
    p_UI->AddCommand("test_token", &Test_GetNextToken,
                     "TEST: Used to test function responsible for splitting command into tokens.");
    p_UI->AddCommand("test_exception", &Test_ExceptionCatch,
                     "TEST: Used to test if exceptions are properly caught by callback caller.");
}