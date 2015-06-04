void startRTSP(int port);

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/filesystem/path.hpp>

extern "C"
{
    #include <libavcodec/avcodec.h>
}

#include "AlloShared/CubemapFace.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        boost::filesystem::path exePath(argv[0]);
        std::cout << "usage: " << exePath.filename().string() << " <RTSP port>" << std::endl;
        return -1;
    }

    avcodec_register_all();

    // Open already created shared memory object.
    // Must have read and write access since we are using mutexes
    // and locking a mutex is a write operation
    boost::interprocess::managed_shared_memory shm =
        boost::interprocess::managed_shared_memory(boost::interprocess::open_only,
                                                   "MySharedMemory");

    cubemap = shm.find<CubemapImpl>("Cubemap").first;

    int port = atoi(argv[1]);
    startRTSP(port);

    return 0;
}