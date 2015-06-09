#pragma once

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/filesystem.hpp>

class Process
{
public:
    Process(std::string id, bool isSelf);
    ~Process();
    
    bool isAlive();
    void join();
    void waitForBirth();
    bool isSelf();
    
private:
    
    boost::filesystem::path lockfilePath;
    boost::interprocess::file_lock* fileLock;
    std::ofstream* lockfile;
};