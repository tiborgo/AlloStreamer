#include <boost/interprocess/sync/scoped_lock.hpp>
#include <fstream>
#include <boost/thread.hpp>

#include "Process.h"

static boost::filesystem::path getLockfilePath(std::string& id)
{
    boost::filesystem::path path = boost::filesystem::temp_directory_path();

    path /= ("." + id);
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
		fileLock = new boost::interprocess::file_lock(lockfilePath.string().c_str());
        fileLock->lock();
    }
}

Process::~Process()
{
    if (isSelf())
    {
        fileLock->unlock();
        delete lockfile;
		// This would clean up resources we used.
		// However, on Windows files will not get removed until
		// the process holding the file is closed.
		// When using the Unity editor the process will not get closed
		// in between tries causing the crash of the process
		// in the next try.
        // boost::filesystem::remove(lockfilePath);
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

		bool result;
		{
			boost::mutex::scoped_lock lock(isAliveMutex);
			boost::interprocess::file_lock fileLock(lockfilePath.string().c_str());
			result = !fileLock.try_lock();
			if (!result)
			{
				fileLock.unlock();
			}
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
            boost::interprocess::file_lock fileLock(lockfilePath.string().c_str());
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

bool Process::timedJoin(const boost::chrono::microseconds& timeout)
{
	if (!isSelf())
	{
		if (boost::filesystem::exists(lockfilePath))
		{
			boost::interprocess::file_lock fileLock(lockfilePath.string().c_str());
			if (fileLock.timed_lock(boost::get_system_time() + boost::posix_time::microseconds(timeout.count())))
			{
				fileLock.unlock();
				return true;
			}
			else
			{
				return false;
			}
		}
	}
}

bool Process::timedWaitForBirth(const boost::chrono::microseconds& timeout)
{
	size_t sleepIntervals = ceil((double)timeout.count() / 100000);
	size_t counter = 0;
	while (!isAlive())
	{
		if (sleepIntervals >= counter)
		{
			return false;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		counter++;
	}
	return true;
}

bool Process::isSelf()
{
    return (fileLock != nullptr);
}
