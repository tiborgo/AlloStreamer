#include <math.h>
#include <stdio.h>
#include <boost/thread.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/offset_ptr.hpp>

#include "AlloShared/config.h"
#include "AlloShared/Process.h"
#include "AlloServer/AlloServer.h"
#include "AlloShared/Cubemap.hpp"
#include "CubemapExtractionPlugin/CubemapExtractionPlugin.h"

static ShmAllocator* shmAllocator = nullptr;
static Process* thisProcess = nullptr;
static Process alloServerProcess(ALLOSERVER_ID, false);
static boost::chrono::system_clock::time_point presentationTime;

boost::interprocess::managed_shared_memory shm;

void allocateSHM(int facesCount, int resolution)
{
    boost::interprocess::shared_memory_object::remove(SHM_NAME);
    
    shm =
    boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create,
                                               SHM_NAME,
                                               resolution * resolution * 4 * facesCount +
                                               sizeof(Cubemap) +
                                               facesCount * sizeof(CubemapFace) +
                                               65536);
    
    shmAllocator = new ShmAllocator(*(new ShmAllocator::BoostShmAllocator(shm.get_segment_manager())));
    
    shm.destroy<Cubemap::Ptr>("Cubemap");
    cubemap = nullptr;
}

void allocateCubemap(std::vector<CubemapFace*>& faces)
{
    cubemap = Cubemap::create(faces, *shmAllocator);
    Cubemap::Ptr cubemapPtr = *shm.construct<Cubemap::Ptr>("Cubemap")(cubemap.get());
}

void releaseSHM()
{
    boost::interprocess::shared_memory_object::remove(SHM_NAME);
    cubemap = nullptr;
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        std::cout << "FakeUnity <faces count> <face resolution> <target FPS>" << std::endl;
        return -1;
    }
    
    int cubemapFacesCount = atoi(argv[1]);
    int resolution = atoi(argv[2]);
    int targetFPS = atoi(argv[3]);
    boost::chrono::milliseconds targetFrameDuration(1000 / targetFPS);
    presentationTime = boost::chrono::system_clock::now();
    
    // Create random pixels to challenge the encoder
    const size_t randomPixelsListCount = 5;
    std::vector<char*> randomPixelsList;
    for (int i = 0; i < randomPixelsListCount; i++)
    {
        char* randomPixels = new char[resolution * resolution * 4];
        for (int j = 0; j < resolution * resolution * 4; j++)
        {
            randomPixels[j] = rand() % 255;
        }
        randomPixelsList.push_back(randomPixels);
    }
    
    // Create and/or open shared memory
    allocateSHM(cubemapFacesCount, resolution);
    std::vector<CubemapFace*> faces;
    for (int i = 0; i < cubemapFacesCount; i++)
    {
        Frame* content = Frame::create(resolution,
                                       resolution,
                                       AV_PIX_FMT_RGB24,
                                       presentationTime,
                                       *shmAllocator);
        faces.push_back(CubemapFace::create(content,
                                            i,
                                            *shmAllocator));
    }
    allocateCubemap(faces);
    
    thisProcess = new Process(CUBEMAPEXTRACTIONPLUGIN_ID, true);
    
    std::cout << "Faking Unity :p (cubemap " << cubemapFacesCount << "x" << resolution << "x" << resolution << ")" << std::endl;
    
    boost::chrono::system_clock::time_point lastTime = boost::chrono::system_clock::now();
    int frames = 0;
    boost::chrono::milliseconds fpsMeasurementPeriod(5000);
    
    unsigned long frameIndex = 0;
    
    while(true)
    {
        presentationTime = boost::chrono::system_clock::now();
        
        for (CubemapFace* face : faces)
        {
            while (!face->getContent()->getMutex().timed_lock(boost::get_system_time() + boost::posix_time::milliseconds(100)))
            {
                if (!alloServerProcess.isAlive())
                {
                    // Reset the mutex otherwise it will block forever
                    // Reset the condition otherwise it my be in inconsistent state
                    // Hacky solution indeed
                    void* mutexAddr     = &face->getContent()->getMutex();
                    void* conditionAddr = &face->getContent()->getNewPixelsCondition();
                    // That's not possible unfortunately since the mutex is locked and abandoned
                    //face->getMutex().~interprocess_mutex();
                    memset(mutexAddr, 0, sizeof(boost::interprocess::interprocess_mutex));
                    memset(conditionAddr, 0, sizeof(boost::interprocess::interprocess_condition));
                    new (mutexAddr)     boost::interprocess::interprocess_mutex;
                    new (conditionAddr) boost::interprocess::interprocess_condition;
                }
            }
            
            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(face->getContent()->getMutex(),
                                                                                           boost::interprocess::accept_ownership);
            
            // Give the encoder some random pixels
            char* randomPixels = randomPixelsList[frameIndex % randomPixelsListCount];
            memcpy(face->getContent()->getPixels(), randomPixels, resolution * resolution * 4);
            
            face->getContent()->setPresentationTime(presentationTime);
            face->getContent()->getNewPixelsCondition().notify_all();
        }
        frames++;
        
        boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();
        boost::chrono::milliseconds frameDuration = boost::chrono::duration_cast<boost::chrono::milliseconds>(now - presentationTime);
        
        if (frameDuration < targetFrameDuration)
        {
            boost::posix_time::milliseconds remainingDuration((targetFrameDuration - frameDuration).count());
            boost::this_thread::sleep(remainingDuration);
        }
        
        boost::chrono::milliseconds period =  boost::chrono::duration_cast<boost::chrono::milliseconds>(now - lastTime);
        
        if (period >= fpsMeasurementPeriod)
        {
            double fps = frames * 1000.0 / period.count();
            std::cout << "Current FPS is " << fps << std::endl;
            frames = 0;
            lastTime = now;
        }
        
        frameIndex++;
    }
    
    delete thisProcess;
    releaseSHM();
    
    return 0;
}