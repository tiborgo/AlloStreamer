#include <boost/interprocess/sync/scoped_lock.hpp>
#include <fstream>
#include <boost/thread.hpp>

#include "Process.h"

static boost::filesystem::path getLockfilePath(std::string& id)
{
    boost::filesystem::path path = boost::filesystem::temp_directory_path();

    path += "." + id;
    return path;
}

Process::Process(std::string id, bool isSelf)
    : lockfilePath(getLockfilePath(id)),
    fileLock(nullptr),
    lockfile(nullptr)
{
    if (isSelf)
    {
        // create and lock file
        lockfile = new  std::ofstream(lockfilePath.c_str());
        *lockfile << id;
        lockfile->flush();
        fileLock = new boost::interprocess::file_lock(lockfilePath.c_str());
        fileLock->lock();
    }
}

Process::~Process()
{
    if (isSelf())
    {
        fileLock->unlock();
        delete lockfile;
        boost::filesystem::remove(lockfilePath);
    }
}

bool Process::isAlive()
{
    if (!isSelf())
    {
        if (!boost::filesystem::exists(lockfilePath))
        {
            return false;
        }
        boost::interprocess::file_lock fileLock(lockfilePath.c_str());
        bool result = !fileLock.try_lock();
        if (!result)
        {
            fileLock.unlock();
        }
        return result;
    }
    else
    {
        return true;
    }
}

void Process::join()
{
    if (!isSelf())
    {
        if (boost::filesystem::exists(lockfilePath))
        {
            boost::interprocess::file_lock fileLock(lockfilePath.c_str());
            boost::interprocess::scoped_lock<boost::interprocess::file_lock> lock(fileLock);
        }
    }
}

void Process::waitForBirth()
{
    while(!isAlive())
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    }
}

bool Process::isSelf()
{
    return (fileLock != nullptr);
}