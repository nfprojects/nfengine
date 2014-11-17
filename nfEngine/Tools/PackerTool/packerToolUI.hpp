#pragma once

#include "packerToolDefines.hpp"
#include "packerToolCommand.hpp"
#include <functional>

const std::string m_helpCommandStr = "help";
const std::string m_exitCommandStr = "exit";

class packerToolUI
{
    typedef std::function<void()> UIFunc;

public:
    packerToolUI();
    ~packerToolUI();

    void MainLoop();

    void AddCommand(const std::string& commandString, const CommandFunc& command);
    void AddCommand(const std::string& commandString, const CommandFunc& command,
                    const std::string& commandHelpMessage);

    void AddBeforeLoopCommand(const UIFunc& command);
    void AddAfterLoopCommand(const UIFunc& command);

private:
    /*struct packerConfig
    {
        std::string s_OutFilePath;
    };*/

    void CreateConfig();
    void ReadConfig();

    void PrintCommands();

    packerToolCommandList mCommandList;

    UIFunc mBeforeLoopCommand;
    UIFunc mAfterLoopCommand;

    //packerConfig m_curConfig;
    FILE* mConfigFile;
    bool mIsUsable;
};
