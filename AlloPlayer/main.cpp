#include <boost/filesystem.hpp>
#include <boost/ref.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "Renderer.hpp"
#include "AlloShared/StatsUtils.hpp"
#include "AlloShared/to_human_readable_byte_count.hpp"
#include "AlloShared/CommandHandler.hpp"
#include "AlloShared/Console.hpp"
#include "AlloShared/Config.hpp"
#include "AlloShared/CommandLine.hpp"
#include "AlloReceiver/RTSPCubemapSourceClient.hpp"
#include "AlloReceiver/AlloReceiver.h"
#include "AlloReceiver/Stats.hpp"
#include "AlloReceiver/H264CubemapSource.h"

#define DEG_DIV_RAD 57.29577951308233
#define RAD_DIV_DEG  0.01745329251994

const unsigned int DEFAULT_SINK_BUFFER_SIZE = 2000000000;

static Stats    stats;
static bool     noDisplay;
static Renderer renderer;
static auto     lastStatsTime = boost::chrono::steady_clock::now();
static std::string statsFormat = AlloReceiver::formatStringMaker();

StereoCubemap* onNextCubemap(CubemapSource* source, StereoCubemap* cubemap)
{
    for (int i = 0; i < cubemap->getEye(0)->getFacesCount(); i++)
    {
        stats.store(StatsUtils::CubemapFace(i, StatsUtils::CubemapFace::DISPLAYED));
    }
    stats.store(StatsUtils::Cubemap());
    return cubemap;
}

void onReceivedNALU(CubemapSource* source, u_int8_t type, size_t size, int face)
{
    //stats.store(StatsUtils::NALU(type, size, face, StatsUtils::NALU::RECEIVED));
}

void onReceivedFrame(CubemapSource* source, u_int8_t type, size_t size, int face)
{
    //stats.store(StatsUtils::Frame(type, size, face, StatsUtils::Frame::RECEIVED));
}

void onDecodedFrame(CubemapSource* source, u_int8_t type, size_t size, int face)
{
    //stats.store(StatsUtils::Frame(type, size, face, StatsUtils::Frame::DECODED));
}

void onColorConvertedFrame(CubemapSource* source, u_int8_t type, size_t size, int face)
{
    stats.store(StatsUtils::Frame(type, size, face, StatsUtils::Frame::COLOR_CONVERTED));
}

void onAddedFrameToCubemap(CubemapSource* source, int face)
{
    //stats.store(StatsUtils::CubemapFace(face, StatsUtils::CubemapFace::ADDED));
}

void setOnScheduledFrameInCubemap(CubemapSource* source, int face)
{
    stats.store(StatsUtils::CubemapFace(face, StatsUtils::CubemapFace::SCHEDULED));
}

void onDisplayedCubemapFace(Renderer* renderer, int face)
{
    stats.store(StatsUtils::CubemapFace(face, StatsUtils::CubemapFace::DISPLAYED));
}

void onDisplayedFrame(Renderer* renderer)
{
    stats.store(StatsUtils::Cubemap());
}

void onDidConnect(RTSPCubemapSourceClient* client, CubemapSource* cubemapSource)
{
    H264CubemapSource* h264CubemapSource = dynamic_cast<H264CubemapSource*>(cubemapSource);
    if (h264CubemapSource)
    {
        h264CubemapSource->setOnReceivedNALU           (boost::bind(&onReceivedNALU,               _1, _2, _3, _4));
        h264CubemapSource->setOnReceivedFrame          (boost::bind(&onReceivedFrame,              _1, _2, _3, _4));
        h264CubemapSource->setOnDecodedFrame           (boost::bind(&onDecodedFrame,               _1, _2, _3, _4));
        h264CubemapSource->setOnColorConvertedFrame    (boost::bind(&onColorConvertedFrame,        _1, _2, _3, _4));
        h264CubemapSource->setOnAddedFrameToCubemap    (boost::bind(&onAddedFrameToCubemap,        _1, _2));
        h264CubemapSource->setOnScheduledFrameInCubemap(boost::bind(&setOnScheduledFrameInCubemap, _1, _2));
    }
    
    if (noDisplay)
    {
        cubemapSource->setOnNextCubemap(boost::bind(&onNextCubemap, _1, _2));
    }
    else
    {
        renderer.setCubemapSource(cubemapSource);
    }
}



int main(int argc, char* argv[])
{
                  noDisplay        = false;
    std::string   url              = "";
    std::string   interfaceAddress = "0.0.0.0";
    unsigned long bufferSize       = DEFAULT_SINK_BUFFER_SIZE;
    bool          matchStereoPairs = false;
    std::string   configFilePath   = "AlloPlayer.config";
    
    std::initializer_list<CommandHandler::Command> generalCommands =
    {
        {
            "gamma-min",
            {"val"},
            [](const std::vector<std::string>& values)
            {
                renderer.setGammaMin(boost::lexical_cast<float>(values[0]));
            }
        },
        {
            "gamma-max",
            {"val"},
            [](const std::vector<std::string>& values)
            {
                renderer.setGammaMax(boost::lexical_cast<float>(values[0]));
            }
        },
        {
            "gamma-pow",
            {"val"},
            [](const std::vector<std::string>& values)
            {
                renderer.setGammaPow(boost::lexical_cast<float>(values[0]));
            }
        },
        {
            "for-rotation",
            {"deg_alpha", "deg_beta", "deg_gamma"},
            [](const std::vector<std::string>& values)
            {
                renderer.setFORRotation(al::Vec3f(boost::lexical_cast<float>(values[0]) * RAD_DIV_DEG,
                                                  boost::lexical_cast<float>(values[1]) * RAD_DIV_DEG,
                                                  boost::lexical_cast<float>(values[2]) * RAD_DIV_DEG));
            }
        },
        {
            "for-angle",
            {"deg"},
            [](const std::vector<std::string>& values)
            {
                renderer.setFORAngle(boost::lexical_cast<float>(values[0]) * RAD_DIV_DEG);
            }
        },
        {
            "rotation",
            {"deg_alpha", "deg_beta", "deg_gamma"},
            [](const std::vector<std::string>& values)
            {
                renderer.setRotation(al::Vec3f(boost::lexical_cast<float>(values[0]) * RAD_DIV_DEG,
                                               boost::lexical_cast<float>(values[1]) * RAD_DIV_DEG,
                                               boost::lexical_cast<float>(values[2]) * RAD_DIV_DEG));
            }
        }
    };
    
    std::initializer_list<CommandHandler::Command> commandLineOnlyCommands =
    {
        {
            "config",
            {"file_path"},
            [&configFilePath](const std::vector<std::string>& values)
            {
                configFilePath = values[0];
            }
        }
    };
    
    std::initializer_list<CommandHandler::Command> configCommandLineOnlyCommands =
    {
        {
            "no-display",
            {},
            [](const std::vector<std::string>& values)
            {
                noDisplay = true;
            }
        },
        {
            "url",
            {"rtsp_url"},
            [&url](const std::vector<std::string>& values)
            {
                url = values[0];
            }
        },
        {
            "interface-address",
            {"ip"},
            [&interfaceAddress](const std::vector<std::string>& values)
            {
                interfaceAddress = values[0];
            }
        },
        {
            "buffer-size",
            {"bytes"},
            [&bufferSize](const std::vector<std::string>& values)
            {
                bufferSize = boost::lexical_cast<unsigned long>(values[0]);
            }
        },
        {
            "match-stereo-pairs",
            {},
            [&matchStereoPairs](const std::vector<std::string>& values)
            {
                matchStereoPairs = true;
            }
        }
    };
    
    std::initializer_list<CommandHandler::Command> consoleOnlyCommands =
    {
        {
            "stats",
            {},
            [](const std::vector<std::string>& values)
            {
                boost::chrono::steady_clock::time_point now = boost::chrono::steady_clock::now();
                std::cout << stats.summary(boost::chrono::duration_cast<boost::chrono::microseconds>(now - lastStatsTime),
                                           AlloReceiver::statValsMaker,
                                           AlloReceiver::postProcessorMaker,
                                           statsFormat);
                lastStatsTime = now;
            }
        },
        {
            "quit",
            {},
            [](const std::vector<std::string>& values)
            {
                exit(0);
            }
        },
        {
            "info",
            {},
            [&bufferSize, &url, &interfaceAddress, &matchStereoPairs](const std::vector<std::string>& values)
            {
                al::Vec3f forRotation = renderer.getFORRotation() * DEG_DIV_RAD;
                al::Vec3f rotation    = renderer.getRotation()    * DEG_DIV_RAD;
                
                std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(1);
                std::cout << "Display:            " << ((noDisplay) ? "no" : "yes") << std::endl;
                std::cout << "RTSP URL:           " << url << std::endl;
                std::cout << "Interface address:  " << interfaceAddress << std::endl;
                std::cout << "Buffer size:        " << to_human_readable_byte_count(bufferSize, false, false) << std::endl;
                std::cout << "Match stereo pairs: " << ((matchStereoPairs) ? "yes" : "no") << std::endl;
                std::cout << "Gamma min:          " << renderer.getGammaMin() << std::endl;
                std::cout << "Gamma max:          " << renderer.getGammaMax() << std::endl;
                std::cout << "Gamma pow:          " << renderer.getGammaPow() << std::endl;
                std::cout << "FOR angle:          " << renderer.getFORAngle() * DEG_DIV_RAD << "°" << std::endl;
                std::cout << "FOR rotation:       " << "α=" << forRotation[0] << "°\t"
                                                    << "β=" << forRotation[1] << "°\t"
                                                    << "γ=" << forRotation[2] << "°" << std::endl;
                std::cout << "Scene rotation:     " << "α=" << rotation[0] << "°\t"
                                                    << "β=" << rotation[1] << "°\t"
                                                    << "γ=" << rotation[2] << "°" << std::endl;
            }
        }
    };
    
    CommandHandler consoleCommandHandler({generalCommands, consoleOnlyCommands});
    CommandHandler configCommandHandler({generalCommands, configCommandLineOnlyCommands});
    CommandHandler commandLineCommandHandler({generalCommands, configCommandLineOnlyCommands, commandLineOnlyCommands});
    
    // The desired behaviour is that command line parameters override config file settings.
    // So, we would run the config file parser first and then the command line parser.
    // However, then we cannot pass the config file path as a command line parameter.
    // The solution here is to run the command line parser twice:
    // before and after the config file parser.
    auto commandLineParseResult = CommandLine::parseCommandLine(commandLineCommandHandler,
                                                                argc,
                                                                argv);
    if (!commandLineParseResult.first)
    {
        std::cerr << commandLineParseResult.second << std::endl;
        std::cerr << commandLineCommandHandler.getCommandHelpString();
        abort();
    }
    
    auto configParseResult = Config::parseConfigFile(configCommandHandler,
                                                     configFilePath);
    if (!configParseResult.first)
    {
        std::cerr << configParseResult.second << std::endl;
        std::cerr << configCommandHandler.getCommandHelpString();
        abort();
    }
    
    commandLineParseResult = CommandLine::parseCommandLine(commandLineCommandHandler,
                                                                argc,
                                                                argv);
    if (!commandLineParseResult.first)
    {
        std::cerr << commandLineParseResult.second << std::endl;
        std::cerr << commandLineCommandHandler.getCommandHelpString();
        abort();
    }
    
    
    if (url == "")
    {
        std::cerr << "No URL specified." << std::endl;
    }
    
    Console console(consoleCommandHandler);
    console.start();
    
    
    RTSPCubemapSourceClient* rtspClient = RTSPCubemapSourceClient::create(url.c_str(),
                                                                          bufferSize,
                                                                          AV_PIX_FMT_YUV420P,
                                                                          matchStereoPairs,
                                                                          interfaceAddress.c_str());
    rtspClient->setOnDidConnect(boost::bind(&onDidConnect, _1, _2));
    rtspClient->connect();
    
    
    
    if (noDisplay)
    {
        std::cout << "network only" << std::endl;
        while(true)
        {
            boost::this_thread::yield();
        }
    }
    else
    {
        renderer.setOnDisplayedCubemapFace(boost::bind(&onDisplayedCubemapFace, _1, _2));
        renderer.setOnDisplayedFrame(boost::bind(&onDisplayedFrame, _1));
        renderer.start(); // does not return
    }
}
