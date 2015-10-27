#pragma once

#include <iostream>
#include <boost/thread.hpp>
#include <boost/any.hpp>

class Console
{
public:
    typedef boost::function<void (const std::vector<std::string>& values)> OnEnteredCommand;
    
    struct ConsoleCommand
    {
        std::string      name;
        size_t           argsCount;
        OnEnteredCommand callback;
    };
    
    Console(std::initializer_list<ConsoleCommand> commands);
    
    void start();
    
private:
    class ReadlineStreambuf : public std::streambuf
    {
    public:
        ReadlineStreambuf(std::streambuf& outStream);
        
    protected:
        virtual std::streamsize xsputn(const char* s, std::streamsize count);
        virtual int overflow(int c = EOF);
        
    private:
        std::streambuf& outStream;
    };
    
    static char* generator(const char* text, int state);
    static char** completion(const char * text , int start, int end);
    
    void runLoop();
    boost::thread runThread;
    ReadlineStreambuf readlineStreambuf;
    std::vector<ConsoleCommand> commands;
    
    static std::vector<ConsoleCommand>* currentCommands;
};