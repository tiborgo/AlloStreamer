#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <regex>
#include <boost/algorithm/string/join.hpp>

#include "Console.hpp"

void * xmalloc (int size)
{
    void *buf;
    
    buf = malloc (size);
    if (!buf) {
        fprintf (stderr, "Error: Out of memory. Exiting.'n");
        exit (1);
    }
    
    return buf;
}

char * dupstr (char* s) {
    char *r;
    
    r = (char*) xmalloc ((strlen (s) + 1));
    strcpy (r, s);
    return (r);
}

char* my_generator(const char* text, int state)
{
    /*static int list_index, len;
    char *name;
    
    if (!state)
    {
        list_index = 0;
        len = strlen (text);
    }
    
    
    while (name = commands[list_index].c_str()) {
        list_index++;
        
        if (strncmp (name, text, len) == 0)
            return (dupstr(name));
    }*/
    
    /* If no names matched, then return NULL. */
    return ((char *)NULL);
    
}

static char** my_completion( const char * text , int start,  int end)
{
    char **matches;
    
    matches = (char **)NULL;
    
    if (start == 0)
        matches = rl_completion_matches ((char*)text, &my_generator);
    else
        rl_bind_key('\t',rl_abort);
    
    return (matches);
    
}

Console::Console(const std::vector<std::string>& commands)
    :
    readlineStreambuf(*std::cout.rdbuf()),
    commands(commands)
{
    
}

void Console::start()
{
    //std::cout.rdbuf(&readlineStreambuf);
    
    runThread = boost::thread(boost::bind(&Console::runLoop, this));
}

void Console::setOnEnteredCommand(const OnEnteredCommand& callback)
{
    onEnteredCommand = callback;
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
    
    rl_attempted_completion_function = my_completion;
    
    std::regex commandRegex("([a-zA-z0-9-]+)(?: ([a-zA-z0-9\\.]+))?");
    
    while ((buf = readline(" >> ")) != NULL)
    {
        //enable auto-complete
        rl_bind_key('\t', rl_complete);
        
        std::smatch commandMatch;
        std::string command(buf);
        if (std::regex_match(command, commandMatch, commandRegex))
        {
            if (std::find(commands.begin(), commands.end(), commandMatch.str(1)) != commands.end())
            {
                if (onEnteredCommand)
                {
                    auto result = onEnteredCommand(commandMatch.str(1), commandMatch.str(2));
                    if (!result.first)
                    {
                        std::cout << "Wrong value. " << result.second << "." << std::endl;
                    }
                }
            }
            else
            {
                std::cout << "Unknown command. Known commands are " <<  boost::algorithm::join(commands, ", ") << "." << std::endl;
            }
        }
        else if (command != "")
        {
            std::cout << "Wrong syntax. Syntax is command [value]." << std::endl;
        }
        
        if (command != "")
        {
            add_history(buf);
        }
    }
    
    free(buf);
}
