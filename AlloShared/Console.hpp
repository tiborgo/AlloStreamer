#pragma once

#include <iostream>
#include <boost/thread.hpp>
#include <boost/any.hpp>

class Console
{
public:
    typedef boost::function<std::pair<bool, std::string> (std::string, std::string value)> OnEnteredCommand;
    
    Console(const std::vector<std::string>& commands);
    
    void start();
    void setOnEnteredCommand(const OnEnteredCommand& callback);
    
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
    
    void runLoop();
    boost::thread runThread;
    ReadlineStreambuf readlineStreambuf;
    OnEnteredCommand onEnteredCommand;
    std::vector<std::string> commands;
};