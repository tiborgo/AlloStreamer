#include <iostream>

#include "CommandHandler.hpp"

CommandHandler::CommandHandler(std::initializer_list<Command> commands)
    :
    commands(commands)
{
}

bool CommandHandler::executeCommand(const std::string& commandName,
                                    const std::vector<std::string>& args)
{
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
                return true;
            }
            catch (const std::exception& e)
            {
                std::cout << "Wrong value. " << e.what() << "." << std::endl;
            }
        }
        else
        {
            std::cout << "Inputted "
                      << args.size() << " args. Expected " << commandIter->argNames.size()
                      << " args for '" << commandIter->name << "'." << std::endl;
        }
        
    }
    else
    {
        std::cout << "'" << commandName <<"' is unknown." << std::endl;
    }
    
    return false;
}

const std::vector<CommandHandler::Command>& CommandHandler::getCommands()
{
    return commands;
}

void CommandHandler::addCommand(const Command& command)
{
    commands.push_back(command);
}

void CommandHandler::printCommandHelp()
{
    std::cout << "Valid commands:" << std::endl;
    for (auto command : this->commands)
    {
        std::cout << "\t" << command.name;
        for (auto argName : command.argNames)
        {
          std::cout << " <" << argName << ">";
        }
        std::cout << std::endl;
    }
}