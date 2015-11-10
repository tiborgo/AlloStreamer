#include <boost/filesystem.hpp>
#include <boost/ref.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <fstream>
#include <osc/OscReceivedElements.h>
#include <osc/OscPacketListener.h>
#include <osc/OscOutboundPacketStream.h>
#include <ip/UdpSocket.h>

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

static Stats         stats;
static Renderer      renderer;
static auto          lastStatsTime    = boost::chrono::steady_clock::now();
static std::string   statsFormat      = AlloReceiver::formatStringMaker();
static bool          noDisplay        = false;
static std::string   url              = "";
static std::string   interfaceAddress = "0.0.0.0";
static unsigned long bufferSize       = DEFAULT_SINK_BUFFER_SIZE;
static bool          matchStereoPairs = false;
static boost::filesystem::path configFilePath;
static bool          robustSyncing    = false;
static size_t        maxFrameMapSize  = 2;
static boost::filesystem::path logPath = ".";
static std::string   participantId;
static std::string   phase;
static std::ofstream setupLogFile;
static std::ofstream ptsLogFile;
static std::ofstream rotationLogFile;
static UdpTransmitSocket* transmitSocket = nullptr;
static uint16_t      oscRotationPort  = 8500;
static std::string   oscRotationAddress;

rapidjson::Document getIdentifierJSON()
{
    auto timeStamp = boost::chrono::system_clock::now().time_since_epoch();
    
    rapidjson::Document document;
    document.SetObject();
    
    document.AddMember("time",
                       rapidjson::Value(boost::chrono::duration_cast<boost::chrono::microseconds>(timeStamp).count()),
                       document.GetAllocator());
    document.AddMember("participantId",
                       rapidjson::Value(participantId.c_str(), participantId.length()),
                       document.GetAllocator());
    document.AddMember("phase",
                       rapidjson::Value(phase.c_str(), phase.length()),
                       document.GetAllocator());
    return document;
}

void log(const rapidjson::Document& document, std::ofstream& logFile)
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    logFile << buffer.GetString() << "," << std::endl;
    logFile.flush();
}

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

void setOnScheduledCubemap(CubemapSource* source, int64_t pts)
{
    rapidjson::Document document = getIdentifierJSON();
    
    document.AddMember("pts",
                       rapidjson::Value(pts),
                       document.GetAllocator());
    
    log(document, ptsLogFile);
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
        h264CubemapSource->setOnScheduledCubemap       (boost::bind(&setOnScheduledCubemap,        _1, _2));
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

void onRotated(Renderer* renderer)
{
    al::Vec3f rotation = renderer->getRotation() * DEG_DIV_RAD;
    
    // Send rotation to Unity so that binoculars perspective can be adapted correctly
    const int OUTPUT_BUFFER_SIZE = 64;
    char buffer[OUTPUT_BUFFER_SIZE];
    osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);
    
    p << osc::BeginBundleImmediate
      << osc::BeginMessage("/rotation")
      << rotation[0]
      << rotation[1]
      << rotation[2]
      << osc::EndMessage
      << osc::EndBundle;
    transmitSocket->Send(p.Data(), p.Size());
    
    // Log rotation
    rapidjson::Document document = getIdentifierJSON();
    
    rapidjson::Value rotationValue;
    rotationValue.SetObject();
    rotationValue.AddMember("x",
                            rapidjson::Value(rotation[0]),
                            document.GetAllocator());
    rotationValue.AddMember("y",
                            rapidjson::Value(rotation[1]),
                            document.GetAllocator());
    rotationValue.AddMember("z",
                            rapidjson::Value(rotation[2]),
                            document.GetAllocator());
    document.AddMember("rotation",
                       rotationValue,
                       document.GetAllocator());
    
    log(document, rotationLogFile);
}

void logSetup()
{
    rapidjson::Document document = getIdentifierJSON();
    
    al::Vec3f forRotation = renderer.getFORRotation() * DEG_DIV_RAD;
    al::Vec3f rotation    = renderer.getRotation()    * DEG_DIV_RAD;
    
    document.AddMember("display",
                       rapidjson::Value(!noDisplay),
                       document.GetAllocator());
    document.AddMember("rtsp-url",
                       rapidjson::Value(url.c_str(), url.length()),
                       document.GetAllocator());
    document.AddMember("buffer-size",
                       rapidjson::Value((uint64_t)bufferSize),
                       document.GetAllocator());
    document.AddMember("match-stereo-pairs",
                       rapidjson::Value(matchStereoPairs),
                       document.GetAllocator());
    document.AddMember("gamma-min",
                       rapidjson::Value(renderer.getGammaMin()),
                       document.GetAllocator());
    document.AddMember("gamma-max",
                       rapidjson::Value(renderer.getGammaMax()),
                       document.GetAllocator());
    document.AddMember("gamma-pow",
                       rapidjson::Value(renderer.getGammaPow()),
                       document.GetAllocator());
    document.AddMember("for-angle",
                       rapidjson::Value(renderer.getFORAngle() * DEG_DIV_RAD),
                       document.GetAllocator());
    rapidjson::Value forRotationValue;
    forRotationValue.SetObject();
    forRotationValue.AddMember("x",
                               rapidjson::Value(forRotation[0]),
                               document.GetAllocator());
    forRotationValue.AddMember("y",
                               rapidjson::Value(forRotation[1]),
                               document.GetAllocator());
    forRotationValue.AddMember("z",
                               rapidjson::Value(forRotation[2]),
                               document.GetAllocator());
    document.AddMember("for-rotation",
                       forRotationValue,
                       document.GetAllocator());
    rapidjson::Value sceneRotationValue;
    sceneRotationValue.SetObject();
    sceneRotationValue.AddMember("x",
                                 rapidjson::Value(rotation[0]),
                                 document.GetAllocator());
    sceneRotationValue.AddMember("y",
                                 rapidjson::Value(rotation[1]),
                                 document.GetAllocator());
    sceneRotationValue.AddMember("z",
                                 rapidjson::Value(rotation[2]),
                                 document.GetAllocator());
    document.AddMember("scene-rotation",
                       sceneRotationValue,
                       document.GetAllocator());
    document.AddMember("rotation-speed",
                       rapidjson::Value(renderer.getRotationSpeed()),
                       document.GetAllocator());
    document.AddMember("robust-syncing",
                       rapidjson::Value(robustSyncing),
                       document.GetAllocator());
    document.AddMember("cubemap-queue-size",
                       rapidjson::Value((uint64_t)maxFrameMapSize),
                       document.GetAllocator());
    document.AddMember("log-path",
                       rapidjson::Value(logPath.string().c_str(), logPath.string().length()),
                       document.GetAllocator());
    document.AddMember("force-mono",
                       rapidjson::Value(renderer.getForceMono()),
                       document.GetAllocator());
    
    log(document, setupLogFile);
}


int main(int argc, char* argv[])
{
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
            {"α°", "β°", "γ°"},
            [](const std::vector<std::string>& values)
            {
                renderer.setFORRotation(al::Vec3f(boost::lexical_cast<float>(values[0]) * RAD_DIV_DEG,
                                                  boost::lexical_cast<float>(values[1]) * RAD_DIV_DEG,
                                                  boost::lexical_cast<float>(values[2]) * RAD_DIV_DEG));
            }
        },
        {
            "for-angle",
            {"°"},
            [](const std::vector<std::string>& values)
            {
                renderer.setFORAngle(boost::lexical_cast<float>(values[0]) * RAD_DIV_DEG);
            }
        },
        {
            "rotation",
            {"α°", "β°", "γ°"},
            [](const std::vector<std::string>& values)
            {
                renderer.setRotation(al::Vec3f(boost::lexical_cast<float>(values[0]) * RAD_DIV_DEG,
                                               boost::lexical_cast<float>(values[1]) * RAD_DIV_DEG,
                                               boost::lexical_cast<float>(values[2]) * RAD_DIV_DEG));
            }
        },
        {
            "rotation-speed",
            {"val"},
            [](const std::vector<std::string>& values)
            {
                renderer.setRotationSpeed(boost::lexical_cast<float>(values[0]));
            }
        },
        {
            "force-mono",
            {},
            [](const std::vector<std::string>& values)
            {
                renderer.setForceMono(true);
            }
        },
    };
    
    CommandHandler configCommandHandler({});
    
    std::initializer_list<CommandHandler::Command> configCommandLineOnlyCommands =
    {
        {
            "config",
            {"file_path"},
            [&configCommandHandler](const std::vector<std::string>& values)
            {
                configFilePath = boost::filesystem::canonical(boost::filesystem::absolute(values[0], configFilePath.parent_path()));
                
                auto configParseResult = Config::parseConfigFile(configCommandHandler,
                                                                 configFilePath.string());
                if (!configParseResult.first)
                {
                    std::cerr << configParseResult.second << std::endl;
                    std::cerr << configCommandHandler.getCommandHelpString();
                    abort();
                }
            }
        },
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
            [](const std::vector<std::string>& values)
            {
                url = values[0];
            }
        },
        {
            "interface-address",
            {"ip"},
            [](const std::vector<std::string>& values)
            {
                interfaceAddress = values[0];
            }
        },
        {
            "buffer-size",
            {"bytes"},
            [](const std::vector<std::string>& values)
            {
                bufferSize = boost::lexical_cast<unsigned long>(values[0]);
            }
        },
        {
            "match-stereo-pairs",
            {},
            [](const std::vector<std::string>& values)
            {
                matchStereoPairs = true;
            }
        },
        {
            "robust-syncing",
            {},
            [](const std::vector<std::string>& values)
            {
                robustSyncing = true;
            }
        },
        {
            "cubemap-queue-size",
            {"size"},
            [](const std::vector<std::string>& values)
            {
                maxFrameMapSize = boost::lexical_cast<size_t>(values[0]);
            }
        },
        {
            "log-path",
            {"path"},
            [](const std::vector<std::string>& values)
            {
                logPath = values[0];
            }
        },
        {
            "participant-id",
            {"string"},
            [](const std::vector<std::string>& values)
            {
                participantId = values[0];
            }
        },
        {
            "phase",
            {"string"},
            [](const std::vector<std::string>& values)
            {
                phase = values[0];
            }
        },
        {
            "osc-rotation-port",
            {"port"},
            [](const std::vector<std::string>& values)
            {
                oscRotationPort = boost::lexical_cast<uint16_t>(values[0]);
            }
        },
        {
            "osc-rotation-address",
            {"port"},
            [](const std::vector<std::string>& values)
            {
                oscRotationAddress = values[0];
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
            [](const std::vector<std::string>& values)
            {
                al::Vec3f forRotation = renderer.getFORRotation() * DEG_DIV_RAD;
                al::Vec3f rotation    = renderer.getRotation()    * DEG_DIV_RAD;
                
                std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(1);
                std::cout << "Display:                " << ((noDisplay) ? "no" : "yes") << std::endl;
                std::cout << "RTSP URL:               " << url << std::endl;
                std::cout << "Interface address:      " << interfaceAddress << std::endl;
                std::cout << "Buffer size:            " << to_human_readable_byte_count(bufferSize, false, false) << std::endl;
                std::cout << "Match stereo pairs:     " << ((matchStereoPairs) ? "yes" : "no") << std::endl;
                std::cout << "Gamma min:              " << renderer.getGammaMin() << std::endl;
                std::cout << "Gamma max:              " << renderer.getGammaMax() << std::endl;
                std::cout << "Gamma pow:              " << renderer.getGammaPow() << std::endl;
                std::cout << "FOR angle:              " << renderer.getFORAngle() * DEG_DIV_RAD << "°" << std::endl;
                std::cout << "FOR rotation:           " << "α=" << forRotation[0] << "°\t"
                                                        << "β=" << forRotation[1] << "°\t"
                                                        << "γ=" << forRotation[2] << "°" << std::endl;
                std::cout << "Scene rotation:         " << "α=" << rotation[0] << "°\t"
                                                        << "β=" << rotation[1] << "°\t"
                                                        << "γ=" << rotation[2] << "°" << std::endl;
                std::cout << "Rotation speed:         " << renderer.getRotationSpeed() << std::endl;
                std::cout << "Face resolutions:       ";
                auto faceResokutions = renderer.getFaceResolutions();
                for (int eye = 0; eye < StereoCubemap::MAX_EYES_COUNT; eye++)
                {
                    for (int face = 0; face < Cubemap::MAX_FACES_COUNT; face++)
                    {
                        auto faceResolution = faceResokutions[eye * Cubemap::MAX_FACES_COUNT + face];
                        std::cout << faceResolution.first << "x" << faceResolution.second << "px ";
                    }
                    std::cout << std::endl << "                    ";
                }
                std::cout << std::endl;
                std::cout << "Robust syncing:         " << ((robustSyncing) ? "yes" : "no") << std::endl;
                std::cout << "Cubemap queue size:     " << maxFrameMapSize << std::endl;
                std::cout << "Force mono:             " << ((renderer.getForceMono()) ? "yes" : "no") << std::endl;
                std::cout << "Log path:               " << logPath << std::endl;
                std::cout << "Participant id:         " << participantId << std::endl;
                std::cout << "Phase:                  " << phase << std::endl;
                std::cout << "OSC rotation port:      " << oscRotationPort << std::endl;
                std::cout << "OSC rotation address:   " << oscRotationAddress << std::endl;
            }
        }
    };
    
    CommandHandler consoleCommandHandler({generalCommands, consoleOnlyCommands});
    configCommandHandler = CommandHandler({generalCommands, configCommandLineOnlyCommands});
    CommandHandler commandLineCommandHandler({generalCommands, configCommandLineOnlyCommands});
    
    auto commandLineParseResult = CommandLine::parseCommandLine(commandLineCommandHandler,
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
        abort();
    }
    
    transmitSocket = new UdpTransmitSocket(IpEndpointName(oscRotationAddress.c_str(), oscRotationPort));
    
    Console console(consoleCommandHandler);
    console.start();
    
    boost::filesystem::path dir(logPath / participantId / phase);
    boost::filesystem::create_directories(dir);
    ptsLogFile.open     ((dir / "PTS.json"     ).string(), std::ofstream::out | std::ofstream::app);
    setupLogFile.open   ((dir / "Setup.json"   ).string(), std::ofstream::out | std::ofstream::app);
    rotationLogFile.open((dir / "Rotation.json").string(), std::ofstream::out | std::ofstream::app);
    
    logSetup();
    
    RTSPCubemapSourceClient* rtspClient = RTSPCubemapSourceClient::create(url.c_str(),
                                                                          bufferSize,
                                                                          AV_PIX_FMT_YUV420P,
                                                                          matchStereoPairs,
                                                                          robustSyncing,
                                                                          maxFrameMapSize,
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
        renderer.setOnRotated(boost::bind(&onRotated, _1));
        renderer.start(); // does not return
    }
}
