#pragma once

#include <iostream>
#include <boost/thread.hpp>

class Console
{
public:
    Console();
    
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
    
    void runLoop();
    boost::thread runThread;
    ReadlineStreambuf readlineStreambuf;
};