#pragma once

#include "FramedSource.hh"
#include "shared.h"
#include <boost/thread/barrier.hpp>
#include <boost/thread/synchronized_value.hpp>
#include "concurrent_queue.h"
#include "AlloShared/CubemapFace.h"

// The following class can be used to define specific encoder parameters

class CubemapFaceSource : public FramedSource {
public:
	static CubemapFaceSource* createNew(UsageEnvironment& env, CubemapFace* face);

public:
    EventTriggerId eventTriggerId;
    // Note that this is defined here to be a static class variable, because this code is intended to illustrate how to
    // encapsulate a *single* device - not a set of devices.
    // You can, however, redefine this to be a non-static member variable.

protected:
	CubemapFaceSource(UsageEnvironment& env, CubemapFace* face);
    // called only by createNew(), or by subclass constructors
	virtual ~CubemapFaceSource();

private:
    // redefined virtual functions:
    virtual void doGetNextFrame();
    //virtual void doStopGettingFrames(); // optional

private:
	static void deliverFrame0(void* clientData);

public:
    void deliverFrame();
    static void addToBuffer(uint8_t* buf, int surfaceSizeInBytes);

private:
	void frameCubemapFace(int index);

	// Stores unencoded frames
	boost::synchronized_value<concurrent_queue<AVFrame*>> frameBuffer;
	// Here unused frames are stored. Included so that we can allocate all the frames at startup
	// and reuse them during runtime
	boost::synchronized_value<concurrent_queue<AVFrame*>> framePool;

	// Stores encoded frames
	boost::synchronized_value<concurrent_queue<AVPacket>> pktBuffer;

    static unsigned referenceCount; // used to count how many instances of this class currently exist
    //DeviceParameters fParams;
    //AVFrame* frame;
    //AVPacket pkt;
    //unsigned char* randomPixels;

	CubemapFace* face;
    //char* name;
    AVCodecContext* codecContext;
    int counter;
    FILE * myfile;
    
    int64_t networkStartSum;
    int64_t networkStopSum;
    
    int64_t networkTotalStart;
    int64_t networkTotalStop;

    void logTimes(int64_t start, int64_t stop);
    
    boost::thread encodeThread;
    void encodeLoop();
    //boost::barrier encodeBarrier;
    bool shutDownEncode;
    unsigned int encodeSeed;
    
    int64_t encodeStartSumBuffer;
    int64_t encodeStopSumBuffer;
    int64_t encodeStartSumFrame;
    int64_t encodeStopSumFrame;
    int64_t encodeStartSumEncode;
    int64_t encodeStopSumEncode;
    int64_t encodeStartSumSync;
    int64_t encodeStopSumSync;
    //static const float encodeReportsPS;
};
