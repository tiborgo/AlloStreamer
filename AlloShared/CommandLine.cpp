#include <regex>
#include <sstream>

#include "CommandLine.hpp"

std::pair<bool, std::string> CommandLine::parseCommandLine(CommandHandler& commandHandler,
                                                           int             argc,
                                                           char*           argv[])
{
    std::string argRegexString("([^\\s]+)");
    std::string commandRegexString("--([a-zA-z0-9-]+)((?: [^\\s^-][^\\s]*)*)");
    std::string entireCommandRegexString("(" + commandRegexString + ")");
    std::string commandLineRegexString("(?:" + entireCommandRegexString + " )*");
    
    std::stringstream commandLineSS;
    for (int i = 1; i < argc; i++)
    {
        commandLineSS << argv[i] << " ";
    }
    
    std::string commandLine(commandLineSS.str());
    
    std::regex argRegex(argRegexString);
    std::regex commandRegex(commandRegexString);
    std::regex entireCommandRegex(entireCommandRegexString);
    std::regex commandLineRegex(commandLineRegexString);
    
    std::string errorString;
    
    std::smatch commandLineMatch;
    if (std::regex_match(commandLine, commandLineMatch, commandLineRegex))
    {
        std::sregex_token_iterator commandIter(commandLine.begin(),
                                               commandLine.end(),
                                               entireCommandRegex,
                                               1);
        
        while (commandIter != std::sregex_token_iterator())
        {
            std::string command(*commandIter);
            std::smatch commandMatch;
            if (std::regex_match(command, commandMatch, commandRegex))
            {
                std::string argsStr = commandMatch.str(2);//.substr(1, commandMatch.str(2).size()-1);
                std::vector<std::string> args(std::sregex_token_iterator(argsStr.begin(),
                                                                         argsStr.end(),
                                                                         argRegex,
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
            else
            {
                errorString = "wrong syntax. Syntax is command=<value>+.";
                break;
            }
            
            commandIter++;
        }
    }
    else if (commandLine != "")
    {
        errorString = "wrong syntax. Syntax is command=<value>+.";
    }
    
    if (errorString.size() > 0)
    {
        std::stringstream resultSS;
        resultSS << "Error in command line: " << errorString;
        return std::make_pair(false, resultSS.str());
    }
    else
    {
        return std::make_pair(true, "");
    }
}
