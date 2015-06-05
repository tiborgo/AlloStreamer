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
    
private:
    boost::interprocess::file_lock fileLock;
    bool isSelf;
    boost::filesystem::path lockfilePath;
    std::ofstream* lockfile;
};