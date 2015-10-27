#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <regex>
#include <boost/algorithm/string/join.hpp>

#include "Console.hpp"

std::vector<Console::ConsoleCommand>* Console::currentCommands = nullptr;

char* Console::generator(const char* text, int state)
{
    static std::vector<ConsoleCommand>::iterator iter;
    
    if (!state)
    {
        iter = currentCommands->begin();
    }
    
    iter = std::find_if(iter, currentCommands->end(),
              [text](const ConsoleCommand& val)
              {
                  std::string prefix(text);
                  std::string comp = val.name.substr(0, prefix.size());
                  bool d = comp == prefix;
                  return d;
              });
    
    if (iter != currentCommands->end())
    {
        char* result = (char*)malloc(iter->name.size() + 1);
        strcpy(result, iter->name.c_str());
        iter++;
        return result;
    }
    else
    {
        return NULL;
    }
}

char** Console::completion(const char * text , int start, int end)
{
    char **matches;
    
    matches = (char **)NULL;
    
    if (start == 0)
        matches = rl_completion_matches ((char*)text, &Console::generator);
    else
        rl_bind_key('\t',rl_abort);
    
    return (matches);
    
}

Console::Console(std::initializer_list<ConsoleCommand> commands)
    :
    readlineStreambuf(*std::cout.rdbuf()),
    commands(commands)
{
    this->commands.push_back(
    {
        "help",
        0,
        [this](const std::vector<std::string>& values)
        {
            std::cout << "Valid commands:" << std::endl;
            for (auto command : this->commands)
            {
                std::vector<std::string> argVec(command.argsCount, "<arg>");
                argVec.insert(argVec.begin(), command.name);
                std::cout << "\t" << boost::algorithm::join(argVec, "\t") << std::endl;
            }
        }
    });
}

void Console::start()
{
    //std::cout.rdbuf(&readlineStreambuf);
    
    runThread = boost::thread(boost::bind(&Console::runLoop, this));
}

Console::ReadlineStreambuf::ReadlineStreambuf(std::streambuf& outStream)
    :
    outStream(outStream)
{
    
}

std::streamsize Console::ReadlineStreambuf::xsputn(const char* s, std::streamsize count)
{
    int saved_point = rl_point;
    char *saved_line = rl_copy_text(0, rl_end);
    rl_save_prompt();
    rl_replace_line("", 0);
    rl_redisplay();
    
    outStream.sputn(s, count);
    
    rl_restore_prompt();
    rl_replace_line(saved_line, 0);
    rl_point = saved_point;
    rl_redisplay();
    free(saved_line);
    
    return count;
}
    
int Console::ReadlineStreambuf::overflow(int c)
{
    int saved_point = rl_point;
    char *saved_line = rl_copy_text(0, rl_end);
    rl_save_prompt();
    rl_replace_line("", 0);
    rl_redisplay();
    
    char ch = c;
    outStream.sputn(&ch, 1);
    
    rl_restore_prompt();
    rl_replace_line(saved_line, 0);
    rl_point = saved_point;
    rl_redisplay();
    free(saved_line);
    
    return 1;
}

void Console::runLoop()
{
    char *buf;
    
    currentCommands = &commands;
    rl_attempted_completion_function = completion;
    
    std::regex commandRegex("([a-zA-z0-9-]+)((?: [^\\s]+)*)");
    std::regex argsRegex("([^\\s]+)");
    
    while ((buf = readline(" >> ")) != NULL)
    {
        //enable auto-complete
        rl_bind_key('\t', rl_complete);
        
        std::smatch commandMatch;
        std::string command(buf);
        if (std::regex_match(command, commandMatch, commandRegex))
        {
            auto commandIter = std::find_if(commands.begin(), commands.end(),
                                         [&commandMatch](const ConsoleCommand& command){
                                            return commandMatch.str(1) == command.name;
                                         });
                                         
            if (commandIter != commands.end())
            {
                std::string argsStr = commandMatch.str(2);
                std::vector<std::string> args(std::sregex_token_iterator(argsStr.begin(), argsStr.end(), argsRegex, 1),
                                              std::sregex_token_iterator());
                
                if (commandIter->argsCount == args.size())
                {
                    try
                    {
                        commandIter->callback(args);
                    }
                    catch (const std::exception& e)
                    {
                        std::cout << "Wrong value. " << e.what() << "." << std::endl;
                    }
                }
                else
                {
                    std::cout << "Input " << args.size() << " args. Expected " << commandIter->argsCount
                        << " number of args for '" << commandIter->name << "'" << std::endl;
                }
                
            }
            else
            {
                std::cout << "Unknown command. Type 'help' for more info." << std::endl;
            }
        }
        else if (command != "")
        {
            std::cout << "Wrong syntax. Syntax is command <value>*." << std::endl;
        }
        
        if (command != "")
        {
            add_history(buf);
        }
    }
    
    free(buf);
}
