#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "Console.hpp"

char* cmd [] ={ "hello", "world", "hell" ,"word", "quit", " " };

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
    static int list_index, len;
    char *name;
    
    if (!state) {
        list_index = 0;
        len = strlen (text);
    }
    
    while (name = cmd[list_index]) {
        list_index++;
        
        if (strncmp (name, text, len) == 0)
            return (dupstr(name));
    }
    
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

Console::Console() : readlineStreambuf(*std::cout.rdbuf())
{
    
}

void Console::start()
{
    //std::cout << "before" << std::endl;
    //auto coutBuf = std::cout.rdbuf();
    //readlineStreambuf = new ReadlineStreambuf(*coutBuf);
    std::cout.rdbuf(&readlineStreambuf);
    //std::cout << "after" << std::endl;
    
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
    
    rl_attempted_completion_function = my_completion;
    
    while((buf = readline("\n >> "))!=NULL) {
        //enable auto-complete
        rl_bind_key('\t',rl_complete);
        
        std::cout << "cmd " << buf << std::endl;
        //printf("cmd [%s]\n",buf);
        if (strcmp(buf,"quit")==0)
            break;
        if (buf[0]!=0)
            add_history(buf);
    }
    
    free(buf);
}
