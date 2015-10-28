#include <iostream>
#include <sstream>

#include "CommandHandler.hpp"

CommandHandler::CommandHandler(std::initializer_list<std::initializer_list<Command> > commandGroups)
{
    for (auto commands : commandGroups)
    {
        this->commands.insert(this->commands.begin(),
                              commands.begin(),
                              commands.end());
    }
}

std::pair<bool, std::string> CommandHandler::executeCommand(const std::string& commandName,
                                                            const std::vector<std::string>& args)
{
    std::stringstream resultSS;
    
    auto commandIter = std::find_if(commands.begin(), commands.end(),
                                    [&commandName](const Command& command){
                                        return commandName == command.name;
                                    });
                                 
    if (commandIter != commands.end())
    {
        if (commandIter->argNames.size() == args.size())
        {
            try
            {
                commandIter->callback(args);
                return std::make_pair(true, "");
            }
            catch (const std::exception& e)
            {
                resultSS << "Wrong value. " << e.what() << ".";
            }
        }
        else
        {
            resultSS << "Inputted "
                     << args.size() << " args. Expected " << commandIter->argNames.size()
                    << " args for '" << commandIter->name << "'.";
        }
    }
    else
    {
        resultSS << "'" << commandName <<"' is unknown.";
    }
    
    return std::make_pair(false, resultSS.str());
}

const std::vector<CommandHandler::Command>& CommandHandler::getCommands()
{
    return commands;
}

void CommandHandler::addCommand(const Command& command)
{
    commands.push_back(command);
}

std::string CommandHandler::getCommandHelpString()
{
    std::stringstream commandHelpSS;
    commandHelpSS << "Valid commands:" << std::endl;
    for (auto command : this->commands)
    {
        commandHelpSS << "\t" << command.name;
        for (auto argName : command.argNames)
        {
          commandHelpSS << " <" << argName << ">";
        }
        commandHelpSS << std::endl;
    }
    return commandHelpSS.str();
}