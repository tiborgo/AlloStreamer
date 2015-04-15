/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 **********/
// "liveMedia"
// Copyright (c) 1996-2013 Live Networks, Inc.  All rights reserved.
// A template for a MediaSource encapsulating an audio/video input device
// 
// NOTE: Sections of this code labeled "%%% TO BE WRITTEN %%%" are incomplete, and needto be written by the programmer
// (depending on the features of the particulardevice).
// C++ header

#ifndef _MYDEVICE_SOURCE_HH
#define _MYDEVICE_SOURCE_HH

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif
#include "shared.h"
#include <boost/thread/barrier.hpp>

// The following class can be used to define specific encoder parameters

class RandomFramedSource : public FramedSource {
public:
    static RandomFramedSource* createNew(UsageEnvironment& env, char* name);

public:
    static EventTriggerId eventTriggerId;
    // Note that this is defined here to be a static class variable, because this code is intended to illustrate how to
    // encapsulate a *single* device - not a set of devices.
    // You can, however, redefine this to be a non-static member variable.

protected:
    RandomFramedSource(UsageEnvironment& env, char* name);
    // called only by createNew(), or by subclass constructors
    virtual ~RandomFramedSource();

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
    static unsigned referenceCount; // used to count how many instances of this class currently exist
    //DeviceParameters fParams;
    AVFrame* frame;
    AVPacket pkt;
    unsigned char* randomPixels;
    char* name;
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
    boost::barrier encodeBarrier;
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
    static const float encodeReportsPS;
};

#endif
