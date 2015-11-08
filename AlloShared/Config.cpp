#include <regex>
#include <fstream>
#include <sstream>

#include "Config.hpp"

std::pair<bool, std::string> Config::parseConfigFile(CommandHandler& commandHandler,
                                                     const std::string& configFilename)
{
    std::ifstream configFile(configFilename);
    
    std::regex commandRegex("([a-zA-z0-9-]+)(?:=((?: ?[^\\s^ ]+)+))?");
    std::regex argsRegex("([^\\s]+)");
    
    std::string errorString;
    
    size_t lineNumber = 1;
    std::string line;
    while (std::getline(configFile, line))
    {
        std::smatch commandMatch;
        std::string command(line);
        if (std::regex_match(command, commandMatch, commandRegex))
        {
            std::string argsStr = commandMatch.str(2);
            std::vector<std::string> args(std::sregex_token_iterator(argsStr.begin(),
                                                                     argsStr.end(),
                                                                     argsRegex,
                                                                     1),
                                          std::sregex_token_iterator());
            
            auto executeResult = commandHandler.executeCommand(commandMatch.str(1),
                                                               args);
            if (!executeResult.first)
            {
                errorString = executeResult.second;
                break;
            }
        }
        else if (line != "")
        {
            errorString = "wrong syntax. Syntax is command=<value>+.";
            break;
        }
        
        lineNumber++;
    }
    
    if (errorString.size() > 0)
    {
        std::stringstream resultSS;
        resultSS << "Error in config file '" << configFilename << "', line " << lineNumber << " '" << line << "': " << errorString;
        return std::make_pair(false, resultSS.str());
    }
    else
    {
        return std::make_pair(true, "");
    }
}
