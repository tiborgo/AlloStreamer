#pragma once

#include <boost/function.hpp>
#include <vector>

class CommandHandler
{
public:
    typedef boost::function<void (const std::vector<std::string>& values)> OnEnteredCommand;
    
    struct Command
    {
        std::string              name;
        std::vector<std::string> argNames;
        OnEnteredCommand         callback;
    };
    
    CommandHandler(std::initializer_list<std::initializer_list<Command> > commandGroups);
    
    std::pair<bool, std::string> executeCommand(const std::string& commandName,
                                                const std::vector<std::string>& args);
    
    const std::vector<Command>& getCommands();
    void addCommand(const Command& command);
    std::string getCommandHelpString();
    
private:
    std::vector<Command> commands;
};