#include "RTSPCubemapSourceClient.hpp"


char const* progName;
UsageEnvironment* env;
struct timeval startTime;
char const* streamURL = NULL;
portNumBits tunnelOverHTTPPortNum = 0;
int verbosityLevel = 1; // by default, print verbose output
char const* clientProtocolName = "RTSP";
char* userAgent = NULL;
MediaSession* session = NULL;
int simpleRTPoffsetArg = -1;
MediaSubsession *subsession;
unsigned fileSinkBufferSize = 1000000;
unsigned socketInputBufferSize = 0;
double initialSeekTime = 0.0f;
char* initialAbsoluteSeekTime = NULL;
double endTime;
TaskToken arrivalCheckTimerTask = NULL;

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

void RTSPCubemapSourceClient::checkForPacketArrival(void* self)
{
	// Check each subsession, to see whether it has received data packets:
	unsigned numSubsessionsChecked = 0;
	unsigned numSubsessionsWithReceivedData = 0;
	unsigned numSubsessionsThatHaveBeenSynced = 0;

	MediaSubsessionIterator iter(*session);
	MediaSubsession* subsession;
	while ((subsession = iter.next()) != NULL)
	{
		RTPSource* src = subsession->rtpSource();
		if (src == NULL) continue;
		++numSubsessionsChecked;

		if (src->receptionStatsDB().numActiveSourcesSinceLastReset() > 0)
		{
			// At least one data packet has arrived
			++numSubsessionsWithReceivedData;
		}
		if (src->hasBeenSynchronizedUsingRTCP())
		{
			++numSubsessionsThatHaveBeenSynced;
		}
	}

	unsigned numSubsessionsToCheck = numSubsessionsChecked;

		struct timeval timeNow;
		gettimeofday(&timeNow, NULL);
		char timestampStr[100];
		sprintf(timestampStr, "%ld%03ld", timeNow.tv_sec, (long)(timeNow.tv_usec / 1000));
		*env << "Data packets have begun arriving [" << timestampStr << "]\007\n";
		return;

	// No luck, so reschedule this check again, after a delay:
	int uSecsToDelay = 100000; // 100 ms
	arrivalCheckTimerTask = env->taskScheduler().scheduleDelayedTask(uSecsToDelay,
		(TaskFunc*)checkForPacketArrival, self);
}

void RTSPCubemapSourceClient::continueAfterDESCRIBE2(RTSPClient* self_, int resultCode, char* resultString)
{
    RTSPCubemapSourceClient* self = (RTSPCubemapSourceClient*)self_;
    
	static int count = 0;
	char* sdpDescription = resultString;
	*env << "Opened URL \"" << streamURL << "\", returning a SDP description:\n" << sdpDescription << "\n";
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
		*env << "Failed to start playing session: " << resultString << "\n";
		delete[] resultString;
		self->shutdown();
		return;
	}
	else
	{
		*env << "Started playing session\n";
	}
	delete[] resultString;

	// Figure out how long to delay (if at all) before shutting down, or
	// repeating the playing

	char const* actionString = "Data is being streamed";
	*env << actionString << "...\n";

	// Watch for incoming packets (if desired):
	checkForPacketArrival(NULL);
	//checkInterPacketGaps(NULL);

	//ourClient->sendDescribeCommand(continueAfterDESCRIBE2);
}


void RTSPCubemapSourceClient::subsessionByeHandler(void* clientData)
{
	struct timeval timeNow;
	gettimeofday(&timeNow, NULL);
	unsigned secsDiff = timeNow.tv_sec - startTime.tv_sec;

	MediaSubsession* subsession = (MediaSubsession*)clientData;
	*env << "Received RTCP \"BYE\" on \"" << subsession->mediumName()
		<< "/" << subsession->codecName()
		<< "\" subsession (after " << secsDiff
		<< " seconds)\n";

	// Act now as if the subsession had closed:
	subsessionAfterPlaying(subsession);
}

void RTSPCubemapSourceClient::createOutputFiles(char const* periodicFilenameSuffix)
{
	char outFileName[1000];

	// Create and start "FileSink"s for each subsession:

	MediaSubsessionIterator iter(*session);
	while ((subsession = iter.next()) != NULL)
	{
		if (subsession->readSource() == NULL) continue; // was not initiated

		// Create an output file for each desired stream:
		// Output file name is
		//     "<filename-prefix><medium_name>-<codec_name>-<counter><periodicFilenameSuffix>"
		static unsigned streamCounter = 0;
		sprintf(outFileName, "%s-%s-%d%s", subsession->mediumName(),
			subsession->codecName(), ++streamCounter, periodicFilenameSuffix);
        
        if (delegate)
        {
            subsession->sink = delegate->getSinkForSubsession(this, subsession);
        }
        else
        {
            subsession->sink = NULL;
        }

		/*H264RawPixelsSink* sink = NULL;
		if (strcmp(subsession->mediumName(), "video") == 0)
		{
			if (strcmp(subsession->codecName(), "H264") == 0)
			{
				// Open window displaying the H.264 video
				sink = H264RawPixelsSink::createNew(*env, fileSinkBufferSize);
                sinks.push_back(sink);
                lastFrames.push_back(NULL);
			}
		}*/
		//subsession->sink = sink;

		if (subsession->sink == NULL)
		{
			*env << "Failed to create FileSink for \"" << outFileName
				<< "\": " << env->getResultMsg() << "\n";
		}
		else
		{
			*env << "Outputting data from the \"" << subsession->mediumName()
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
	static MediaSubsessionIterator* setupIter = NULL;
	if (setupIter == NULL) setupIter = new MediaSubsessionIterator(*session);
	while ((subsession = setupIter->next()) != NULL)
	{
		// We have another subsession left to set up:
		if (subsession->clientPortNum() == 0) continue; // port # was not set

		sendSetupCommand(*subsession, continueAfterSETUP);

		return;
	}

	// We're done setting up subsessions.
	delete setupIter;

	// Create output files:
	createOutputFiles("");

	// Finally, start playing each subsession, to start the data flow:
	double duration = session->playEndTime() - initialSeekTime; // use SDP end time

	endTime = initialSeekTime;
	endTime = -1.0f;


	char const* absStartTime = initialAbsoluteSeekTime != NULL ? initialAbsoluteSeekTime : session->absStartTime();
	if (absStartTime != NULL)
	{
		// Either we or the server have specified that seeking should be done by 'absolute' time:
		sendPlayCommand(*session, continueAfterPLAY, absStartTime, session->absEndTime(), 1.0);
	}
	else
	{
		// Normal case: Seek by relative time (NPT):
		sendPlayCommand(*session, continueAfterPLAY, initialSeekTime, endTime, 1.0);
	}
}

void RTSPCubemapSourceClient::continueAfterSETUP(RTSPClient* self_, int resultCode, char* resultString)
{
    RTSPCubemapSourceClient* self = (RTSPCubemapSourceClient*)self_;
    
	if (resultCode == 0)
	{
		*env << "Setup \"" << subsession->mediumName()
			<< "/" << subsession->codecName()
			<< "\" subsession (";
		if (subsession->rtcpIsMuxed())
		{
			*env << "client port " << subsession->clientPortNum();
		}
		else
		{
			*env << "client ports " << subsession->clientPortNum()
				<< "-" << subsession->clientPortNum() + 1;
		}
		*env << ")\n";
	}
	else
	{
		*env << "Failed to setup \"" << subsession->mediumName()
			<< "/" << subsession->codecName()
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
		*env << "Failed to get a SDP description for the URL \"" << streamURL << "\": " << resultString << "\n";
		delete[] resultString;
		self->shutdown();
	}

	char* sdpDescription = resultString;
	*env << "Opened URL \"" << streamURL << "\", returning a SDP description:\n" << sdpDescription << "\n";

	// Create a media session object from this SDP description:
	session = MediaSession::createNew(*env, sdpDescription);
	delete[] sdpDescription;
	if (session == NULL)
	{
		*env << "Failed to create a MediaSession object from the SDP description: " << env->getResultMsg() << "\n";
		self->shutdown();
	}
	else if (!session->hasSubsessions())
	{
		*env << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
		self->shutdown();
	}

	// Then, setup the "RTPSource"s for the session:
	MediaSubsessionIterator iter(*session);
	MediaSubsession *subsession;
	Boolean madeProgress = False;
	while ((subsession = iter.next()) != NULL)
	{

		if (!subsession->initiate(simpleRTPoffsetArg))
		{
			*env << "Unable to create receiver for \"" << subsession->mediumName()
				<< "/" << subsession->codecName()
				<< "\" subsession: " << env->getResultMsg() << "\n";
		}
		else
		{
			*env << "Created receiver for \"" << subsession->mediumName()
				<< "/" << subsession->codecName() << "\" subsession (";
			if (subsession->rtcpIsMuxed())
			{
				*env << "client port " << subsession->clientPortNum();
			}
			else
			{
				*env << "client ports " << subsession->clientPortNum()
					<< "-" << subsession->clientPortNum() + 1;
			}
			*env << ")\n";
			madeProgress = True;

			

			if (subsession->rtpSource() != NULL)
			{
				// Because we're saving the incoming data, rather than playing
				// it in real time, allow an especially large time threshold
				// (1 second) for reordering misordered incoming packets:
				unsigned const thresh = 1; // 1 second
				subsession->rtpSource()->setPacketReorderingThresholdTime(thresh);

				// Set the RTP source's OS socket buffer size as appropriate - either if we were explicitly asked (using -B),
				// or if the desired FileSink buffer size happens to be larger than the current OS socket buffer size.
				// (The latter case is a heuristic, on the assumption that if the user asked for a large FileSink buffer size,
				// then the input data rate may be large enough to justify increasing the OS socket buffer size also.)
				int socketNum = subsession->rtpSource()->RTPgs()->socketNum();
				unsigned curBufferSize = getReceiveBufferSize(*env, socketNum);
				if (socketInputBufferSize > 0 || fileSinkBufferSize > curBufferSize)
				{
					unsigned newBufferSize = socketInputBufferSize > 0 ? socketInputBufferSize : fileSinkBufferSize;
					newBufferSize = setReceiveBufferTo(*env, socketNum, newBufferSize);
					if (socketInputBufferSize > 0)
					{ // The user explicitly asked for the new socket buffer size; announce it:
						*env << "Changed socket receive buffer size for the \""
							<< subsession->mediumName()
							<< "/" << subsession->codecName()
							<< "\" subsession from "
							<< curBufferSize << " to "
							<< newBufferSize << " bytes\n";
					}
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

	// Perform additional 'setup' on each subsession, before playing them:
	self->setupStreams();
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
	env->taskScheduler().doEventLoop(); // does not return
}

void RTSPCubemapSourceClient::connect()
{
    networkThread = boost::thread(boost::bind(&RTSPCubemapSourceClient::networkLoop, this));
}

RTSPCubemapSourceClient* RTSPCubemapSourceClient::createNew(char const* rtspURL,
                                                            int verbosityLevel,
                                                            char const* applicationName,
                                                            portNumBits tunnelOverHTTPPortNum,
                                                            int socketNumToServer)
{
    // Begin by setting up our usage environment:
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    ::env = BasicUsageEnvironment::createNew(*scheduler);
    
    return new RTSPCubemapSourceClient(*env,
                                       rtspURL,
                                       verbosityLevel,
                                       applicationName,
                                       tunnelOverHTTPPortNum,
                                       socketNumToServer);
}

RTSPCubemapSourceClient::RTSPCubemapSourceClient(UsageEnvironment& env,
                                                 char const* rtspURL,
                                                 int verbosityLevel,
                                                 char const* applicationName,
                                                 portNumBits tunnelOverHTTPPortNum,
                                                 int socketNumToServer)
    :
    RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, socketNumToServer)
{
    gettimeofday(&startTime, NULL);
    
    streamURL = rtspURL;
    
    setUserAgentString(userAgent);
}
