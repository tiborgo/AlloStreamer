#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>

#include "H264RawPixelsSink.h"
#include "H264CubemapSource.h"
#include "RTSPCubemapSourceClient.hpp"

void RTSPCubemapSourceClient::setOnDidConnect(const std::function<void (RTSPCubemapSourceClient*, CubemapSource*)>& onDidConnect)
{
    this->onDidConnect = onDidConnect;
}

void RTSPCubemapSourceClient::shutdown(int exitCode)
{
}

void RTSPCubemapSourceClient::subsessionAfterPlaying(void* clientData)
{
	// Begin by closing this media subsession's stream:
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	// Next, check whether *all* subsessions' streams have now been closed:
	MediaSession& session = subsession->parentSession();
	MediaSubsessionIterator iter(session);
	while ((subsession = iter.next()) != NULL)
	{
		if (subsession->sink != NULL) return; // this subsession is still active
	}

	// All subsessions' streams have now been closed
	//sessionAfterPlaying();
}

void RTSPCubemapSourceClient::checkForPacketArrival(void* self_)
{
 //   RTSPCubemapSourceClient* self = (RTSPCubemapSourceClient*)self_;
 //   
	//// Check each subsession, to see whether it has received data packets:
	//unsigned numSubsessionsChecked = 0;
	//unsigned numSubsessionsWithReceivedData = 0;
	//unsigned numSubsessionsThatHaveBeenSynced = 0;

	//MediaSubsessionIterator iter(*self->session);
	//MediaSubsession* subsession;
	//while ((subsession = iter.next()) != NULL)
	//{
	//	RTPSource* src = subsession->rtpSource();
	//	if (src == NULL) continue;
	//	++numSubsessionsChecked;

	//	if (src->receptionStatsDB().numActiveSourcesSinceLastReset() > 0)
	//	{
	//		// At least one data packet has arrived
	//		++numSubsessionsWithReceivedData;
	//	}
	//	if (src->hasBeenSynchronizedUsingRTCP())
	//	{
	//		++numSubsessionsThatHaveBeenSynced;
	//	}
	//}

	//unsigned numSubsessionsToCheck = numSubsessionsChecked;

	//	struct timeval timeNow;
	//	gettimeofday(&timeNow, NULL);
	//	char timestampStr[100];
	//	sprintf(timestampStr, "%ld%03ld", timeNow.tv_sec, (long)(timeNow.tv_usec / 1000));
	//	self->envir() << "Data packets have begun arriving [" << timestampStr << "]\007\n";
	//	return;

	//// No luck, so reschedule this check again, after a delay:
	//int uSecsToDelay = 100000; // 100 ms
	//TaskToken arrivalCheckTimerTask = self->session->envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
	//	(TaskFunc*)checkForPacketArrival, self);
}

void RTSPCubemapSourceClient::continueAfterDESCRIBE2(RTSPClient* self_, int resultCode, char* resultString)
{
    RTSPCubemapSourceClient* self = (RTSPCubemapSourceClient*)self_;
    
	static int count = 0;
	char* sdpDescription = resultString;
	//self->envir() << "Opened URL \"" << self->url() << "\", returning a SDP description:\n" << sdpDescription << "\n";
	if (count == 0)
	{
		self->sendDescribeCommand(continueAfterDESCRIBE2);
	}
	count++;
}

void RTSPCubemapSourceClient::continueAfterPLAY(RTSPClient* self_, int resultCode, char* resultString)
{
    RTSPCubemapSourceClient* self = (RTSPCubemapSourceClient*)self_;
    
	if (resultCode != 0)
	{
		self->envir() << "Failed to start playing session: " << resultString << "\n";
		delete[] resultString;
		self->shutdown();
		return;
	}
	else
	{
		self->envir() << "Started playing session\n";
	}
	delete[] resultString;

	// Figure out how long to delay (if at all) before shutting down, or
	// repeating the playing

	char const* actionString = "Data is being streamed";
	self->envir() << actionString << "...\n";

	// Watch for incoming packets (if desired):
	checkForPacketArrival(self);
	//checkInterPacketGaps(NULL);

	//ourClient->sendDescribeCommand(continueAfterDESCRIBE2);
}

void RTSPCubemapSourceClient::periodicQOSMeasurement(void* self_)
{
    RTSPCubemapSourceClient* self = (RTSPCubemapSourceClient*)self_;
    double totalKBytes = 0.0;
    unsigned int totalPacketsReceived = 0;
    unsigned int totalPacketsExpected = 0;
    for (MediaSubsession* subsession : self->subsessions)
    {
        RTPSource* src = subsession->rtpSource();
        RTPReceptionStatsDB::Iterator statsIter(src->receptionStatsDB());
        RTPReceptionStats* stats;
        while ((stats = statsIter.next(True)) != NULL)
        {
            totalKBytes += stats->totNumKBytesReceived();
            totalPacketsReceived += stats->totNumPacketsReceived();
            totalPacketsExpected += stats->totNumPacketsExpected();
        }
    }
    
    double totalKBytesInInterval = totalKBytes - self->lastTotalKBytes;
    unsigned int totalPacketsReceivedInInterval = totalPacketsReceived - self->lastTotalPacketsReceived;
    unsigned int totalPacketsExpectedInInterval = totalPacketsExpected - self->lastTotalPacketsExpected;
    
    self->lastTotalKBytes = totalKBytes;
    self->lastTotalPacketsReceived = totalPacketsReceived;
    self->lastTotalPacketsExpected = totalPacketsExpected;
    
    unsigned int totalPacketsLostInInterval = totalPacketsExpectedInInterval - totalPacketsReceivedInInterval;
    
    std::cout << "Client: " << std::setprecision(10) << std::setiosflags(std::ios::fixed) << std::setprecision(1) <<totalKBytesInInterval/10 * 8 / 1000 << " MBit/s; " <<
        " packets received: " << totalPacketsReceivedInInterval << "; packets lost: " << totalPacketsLostInInterval <<
        "; packet loss: " << std::setprecision(2) << (double)totalPacketsLostInInterval / totalPacketsReceivedInInterval * 100.0 << "%" << std::endl;
    
    self->envir().taskScheduler().scheduleDelayedTask(10000000, (TaskFunc*)RTSPCubemapSourceClient::periodicQOSMeasurement, self);
}

void RTSPCubemapSourceClient::subsessionByeHandler(void* clientData)
{
	struct timeval timeNow;
	gettimeofday(&timeNow, NULL);

	MediaSubsession* subsession = (MediaSubsession*)clientData;
	subsession->sink->envir() << "Received RTCP \"BYE\" on \"" << subsession->mediumName()
		<< "/" << subsession->codecName()
		<< "\" subsession\n";

	// Act now as if the subsession had closed:
	subsessionAfterPlaying(subsession);
}

void RTSPCubemapSourceClient::createOutputFiles(char const* periodicFilenameSuffix)
{
	char outFileName[1000];

	// Create and start "FileSink"s for each subsession:
    
    /*std::vector<MediaSubsession*> subsessions;
	for (MediaSession* session : sessions)
	{
		MediaSubsessionIterator iter(*session);
		MediaSubsession* subsession = iter.next();
		if (subsession->readSource() == NULL) continue; // was not initiated

		// Create an output file for each desired stream:
		// Output file name is
		//     "<filename-prefix><medium_name>-<codec_name>-<counter><periodicFilenameSuffix>"
		static unsigned streamCounter = 0;
		sprintf(outFileName, "%s-%s-%d%s", subsession->mediumName(),
			subsession->codecName(), ++streamCounter, periodicFilenameSuffix);
        
        subsessions.push_back(subsession);
    }*/
    
    // Create CubemapSource based on discovered stream
    bool isH264 = true;
    for (MediaSubsession* subsession : subsessions)
    {
        if (strcmp(subsession->mediumName(), "video") != 0 ||
            strcmp(subsession->codecName(), "H264") != 0)
        {
            isH264 = false;
        }
    }
    
    if (isH264)
    {
        std::vector<H264RawPixelsSink*> h264Sinks;
        std::vector<MediaSink*> sinks;
        for (int i = 0; i < (std::min)(subsessions.size(), (size_t)(StereoCubemap::MAX_EYES_COUNT * Cubemap::MAX_FACES_COUNT)); i++)
        {
            H264RawPixelsSink* sink = H264RawPixelsSink::createNew(envir(),
                                                                   sinkBufferSize,
                                                                   format,
                                                                   subsessions[i]);
            subsessions[i]->sink = sink;
            
            h264Sinks.push_back(sink);
            sinks.push_back(sink);
        }
        
        if (onDidConnect)
        {
            onDidConnect(this, new H264CubemapSource(h264Sinks, format, matchStereoPairs));
        }
    }
    
    for (MediaSubsession* subsession : subsessions)
    {
		if (subsession->sink == NULL)
		{
			envir() << "Failed to create FileSink for \"" << outFileName
				<< "\": " << subsession->parentSession().envir().getResultMsg() << "\n";
		}
		else
		{
			envir() << "Outputting data from the \"" << subsession->mediumName()
				<< "/" << subsession->codecName()
				<< "\" subsession to \"" << outFileName << "\"\n";

			subsession->sink->startPlaying(*(subsession->readSource()),
				subsessionAfterPlaying,
				subsession);

			// Also set a handler to be called if a RTCP "BYE" arrives
			// for this subsession:
			if (subsession->rtcpInstance() != NULL)
			{
				subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, subsession);
			}
		}
	}
}



void RTSPCubemapSourceClient::setupStreams()
{
	//static MediaSubsessionIterator* setupIter = NULL;
	static std::vector<MediaSubsession*>::iterator setupIter = subsessions.begin();
	//if (setupIter == NULL) setupIter = new MediaSubsessionIterator(*session);

	//for (MediaSubsession* subsession : subsessions)
//	{
		while (setupIter != subsessions.end())
		{
			subsession = *setupIter;
		// We have another subsession left to set up:
		if (subsession->clientPortNum() == 0) continue; // port # was not set

		//this->subsession = subsession;
		sendSetupCommand(*subsession, continueAfterSETUP);
			
		setupIter++;

			return;
		}

		// We're done setting up subsessions.
		//delete setupIter;

//	}

			// Create output files:
			createOutputFiles("");

			for (MediaSubsession* subsession : subsessions)
			{
				// Finally, start playing each subsession, to start the data flow:
				double initialSeekTime = 0.0f;
				double duration = subsession->parentSession().playEndTime() - initialSeekTime; // use SDP end time

				double endTime;
				endTime = initialSeekTime;
				endTime = -1.0f;

				char* initialAbsoluteSeekTime = NULL;
				char const* absStartTime = initialAbsoluteSeekTime != NULL ? initialAbsoluteSeekTime : subsession->parentSession().absStartTime();
				if (absStartTime != NULL)
				{
					// Either we or the server have specified that seeking should be done by 'absolute' time:
					sendPlayCommand(subsession->parentSession(), continueAfterPLAY, absStartTime, subsession->parentSession().absEndTime(), 1.0);
				}
				else
				{
					// Normal case: Seek by relative time (NPT):
					sendPlayCommand(subsession->parentSession(), continueAfterPLAY, initialSeekTime, endTime, 1.0);
				}
			}
}

void RTSPCubemapSourceClient::continueAfterSETUP(RTSPClient* self_, int resultCode, char* resultString)
{
	static int setups = 0;
	setups++;

    RTSPCubemapSourceClient* self = (RTSPCubemapSourceClient*)self_;
    
	if (resultCode == 0)
	{
		self->envir() << "Setup \"" << self->subsession->mediumName()
			<< "/" << self->subsession->codecName()
			<< "\" subsession (";
		if (self->subsession->rtcpIsMuxed())
		{
			self->envir() << "client port " << self->subsession->clientPortNum();
		}
		else
		{
			self->envir() << "client ports " << self->subsession->clientPortNum()
				<< "-" << self->subsession->clientPortNum() + 1;
		}
		self->envir() << ")\n";
	}
	else
	{
        self->envir() << "Failed to setup \"" << self->subsession->mediumName()
			<< "/" << self->subsession->codecName()
			<< "\" subsession: " << resultString << "\n";
	}
	delete[] resultString;
	
	// Set up the next subsession, if any:
	self->setupStreams();
}

void RTSPCubemapSourceClient::continueAfterDESCRIBE(RTSPClient* self_, int resultCode, char* resultString)
{
    RTSPCubemapSourceClient* self = (RTSPCubemapSourceClient*)self_;
    
	if (resultCode != 0)
	{
		self->envir() << "Failed to get a SDP description for the URL \"" << self->url() << "\": " << resultString << "\n";
		delete[] resultString;
		self->shutdown();
	}

	char* sdpDescription = resultString;
	//self->envir() << "Opened URL \"" << self->url() << "\", returning a SDP description:\n" << sdpDescription << "\n";

	// Parse SDP description and extract subsession information
	std::vector<std::string> sdpLines;
	boost::algorithm::split_regex(sdpLines, sdpDescription, boost::regex("\nm="));
	std::string header = sdpLines[0];
	sdpLines.erase(sdpLines.begin());
	std::transform(sdpLines.begin(), sdpLines.end(), sdpLines.begin(), [](std::string &subsession){ return "m=" + subsession + "\n"; });
	delete[] sdpDescription;

	for (int i = 0; i < sdpLines.size(); i++)
	{

		// Create a media session object from this SDP description:
		TaskScheduler* scheduler = BasicTaskScheduler::createNew();
		BasicUsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
		self->envs.push_back(env);
		MediaSession* session = MediaSession::createNew(*env, (header + sdpLines[i]).c_str());

		std::cout << "created session" << std::endl;
		
		if (session == NULL)
		{
			self->envir() << "Failed to create a MediaSession object from the SDP description: "
				<< env->getResultMsg() << "\n";
			self->shutdown();
		}
		else if (!session->hasSubsessions())
		{
			self->envir() << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
			self->shutdown();
		}

		// Then, setup the "RTPSource"s for the session:
		MediaSubsessionIterator iter(*session);
		MediaSubsession *subsession;
		Boolean madeProgress = False;
		while ((subsession = iter.next()) != NULL)
		{
			self->subsessions.push_back(subsession);

			if (!subsession->initiate())
			{
				self->envir() << "Unable to create receiver for \"" << subsession->mediumName()
					<< "/" << subsession->codecName()
					<< "\" subsession: " << env->getResultMsg() << "\n";
			}
			else
			{
				self->envir() << "Created receiver for \"" << subsession->mediumName()
					<< "/" << subsession->codecName() << "\" subsession (";
				if (subsession->rtcpIsMuxed())
				{
					self->envir() << "client port " << subsession->clientPortNum();
				}
				else
				{
					self->envir() << "client ports " << subsession->clientPortNum()
						<< "-" << subsession->clientPortNum() + 1;
				}
				self->envir() << ")\n";
				madeProgress = True;

			

				if (subsession->rtpSource() != NULL)
				{
					// Because we're saving the incoming data, rather than playing
					// it in real time, allow an especially large time threshold
					// (1 second) for reordering misordered incoming packets:
					unsigned const thresh = 100000; // 1 second
					subsession->rtpSource()->setPacketReorderingThresholdTime(thresh);

					// Set the RTP source's OS socket buffer size as appropriate - either if we were explicitly asked (using -B),
					// or if the desired FileSink buffer size happens to be larger than the current OS socket buffer size.
					// (The latter case is a heuristic, on the assumption that if the user asked for a large FileSink buffer size,
					// then the input data rate may be large enough to justify increasing the OS socket buffer size also.)
					int socketNum = subsession->rtpSource()->RTPgs()->socketNum();
					unsigned curBufferSize = getReceiveBufferSize(*env, socketNum);
					if (self->sinkBufferSize > curBufferSize)
					{
						unsigned newBufferSize = self->sinkBufferSize;
						newBufferSize = setReceiveBufferTo(*env, socketNum, newBufferSize);
					}
				}
				//		}
				//	}
				//	else
				//	{
				//		if (subsession->clientPortNum() == 0)
				//		{
				//			*env << "No client port was specified for the \""
				//				<< subsession->mediumName()
				//				<< "/" << subsession->codecName()
				//				<< "\" subsession.  (Try adding the \"-p <portNum>\" option.)\n";
				//		}
				//		else
				//		{
				//			madeProgress = True;
				//		}
			}
		}
		//if (!madeProgress) shutdown();

	}

	// Perform additional 'setup' on each subsession, before playing them:
	self->setupStreams();
    
    self->envir().taskScheduler().scheduleDelayedTask(10000000, (TaskFunc*)RTSPCubemapSourceClient::periodicQOSMeasurement, self);

	for (int i = 0; i < self->envs.size(); i++)
	{
		boost::thread* abc = new boost::thread(boost::bind(&TaskScheduler::doEventLoop, &self->envs[i]->taskScheduler(), nullptr));
		self->sessionThreads.push_back(boost::shared_ptr<boost::thread>(abc));
	}
}

void RTSPCubemapSourceClient::continueAfterOPTIONS(RTSPClient* self_, int resultCode, char* resultString)
{
    RTSPCubemapSourceClient* self = (RTSPCubemapSourceClient*)self_;
	delete[] resultString;

	// Next, get a SDP description for the stream:
	self->sendDescribeCommand(continueAfterDESCRIBE);
}

void RTSPCubemapSourceClient::networkLoop()
{
    // Begin by sending an "OPTIONS" command:
    sendOptionsCommand(continueAfterOPTIONS);
    
	// All subsequent activity takes place within the event loop:
	envir().taskScheduler().doEventLoop(); // does not return
}

void RTSPCubemapSourceClient::connect()
{
    networkThread = boost::thread(boost::bind(&RTSPCubemapSourceClient::networkLoop, this));
}

RTSPCubemapSourceClient* RTSPCubemapSourceClient::create(char const* rtspURL,
                                                         unsigned int sinkBufferSize,
                                                         AVPixelFormat format,
                                                         bool matchStereoPairs,
                                                         const char* interfaceAddress,
                                                         int verbosityLevel,
                                                         char const* applicationName,
                                                         portNumBits tunnelOverHTTPPortNum,
                                                         int socketNumToServer)
{
    NetAddressList addresses(interfaceAddress);
    if (addresses.numAddresses() == 0)
    {
        std::cout << "Inteface \"" << interfaceAddress << "\" does not exist" << std::endl;
        return nullptr;
    }
    ReceivingInterfaceAddr = *(unsigned*)(addresses.firstAddress()->data());
    
    // Begin by setting up our usage environment:
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    BasicUsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
    
    return new RTSPCubemapSourceClient(*env,
                                       rtspURL,
                                       sinkBufferSize,
                                       format,
                                       matchStereoPairs,
                                       verbosityLevel,
                                       applicationName,
                                       tunnelOverHTTPPortNum,
                                       socketNumToServer);
}

RTSPCubemapSourceClient::RTSPCubemapSourceClient(UsageEnvironment& env,
                                                 char const* rtspURL,
                                                 unsigned int sinkBufferSize,
                                                 AVPixelFormat format,
                                                 bool matchStereoPairs,
                                                 int verbosityLevel,
                                                 char const* applicationName,
                                                 portNumBits tunnelOverHTTPPortNum,
                                                 int socketNumToServer)
    :
    RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, socketNumToServer),
    sinkBufferSize(sinkBufferSize), format(format), lastTotalKBytes(0.0), lastTotalPacketsReceived(0), lastTotalPacketsExpected(0),
    matchStereoPairs(matchStereoPairs)
{
}
