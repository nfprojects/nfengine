#pragma once

#include "packerToolDefines.hpp"
#include "packerToolCommand.hpp"

const std::string m_helpCommandStr = "help";
const std::string m_exitCommandStr = "exit";

class packerToolUI
{
    typedef void (*UIFunc)();

public:
    packerToolUI();
    ~packerToolUI();

    void MainLoop();

    void AddCommand(std::string commandString, CommandFunc command);
    void AddCommand(std::string commandString, CommandFunc command, std::string commandHelpMessage);

    void AddBeforeLoopCommand(UIFunc command);
    void AddAfterLoopCommand(UIFunc command);

private:
    /*struct packerConfig
    {
        std::string s_OutFilePath;
    };*/

    void CreateConfig();
    void ReadConfig();

    void PrintCommands();

    packerToolCommandList m_commandList;

    UIFunc m_beforeLoopCommand;
    UIFunc m_afterLoopCommand;

    //packerConfig m_curConfig;
    FILE* m_configFile;
    bool m_isUsable;
};