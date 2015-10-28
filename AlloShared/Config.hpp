#pragma once

#include <iostream>

#include "CommandHandler.hpp"

class Config
{
public:
    static std::pair<bool, std::string> parseConfigFile(CommandHandler& commandHandler,
                                                        const std::string& configFilename);
    
private:
    Config() {}
};