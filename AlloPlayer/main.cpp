

#include <boost/thread.hpp>

int mainCubemapsSkyboxOptimized();
int mainStaticCubemapBackgroundApp(int argc, char* argv[]);
int mainDynamicCubemapBackgroundApp(int argc, char* argv[]);
int mainCubemap();
int mainAlloPlayer(int argc, char** argv);

int main(int argc, char* argv[])
{

    //mainCubemapsSkyboxOptimized();
    //boost::thread thread(boost::bind(&mainStaticCubemapBackgroundApp, argc, argv));
    //thread.join();
    //mainStaticCubemapBackgroundApp(argc, argv);
    //mainDynamicCubemapBackgroundApp(argc, argv);
    //mainCubemap();
    mainAlloPlayer(argc, argv);
}
