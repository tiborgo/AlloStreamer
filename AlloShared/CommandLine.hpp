#pragma once

#include <iostream>

#include "CommandHandler.hpp"

class CommandLine
{
public:
    static std::pair<bool, std::string> parseCommandLine(CommandHandler& commandHandler,
                                                         int             argc,
                                                         char*           argv[]);
    
private:
    CommandLine() {}
};