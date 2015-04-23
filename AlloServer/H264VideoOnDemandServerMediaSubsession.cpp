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
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a H264 video file.
// Implementation

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#include "H264VideoOnDemandServerMediaSubsession.h"
#include "H264VideoRTPSink.hh"
#include "RandomFramedSource.h"
#include "CubemapFaceSource.h"
#include "H264VideoStreamDiscreteFramer.hh"
UsageEnvironment* envi;

H264VideoOnDemandServerMediaSubsession*
H264VideoOnDemandServerMediaSubsession::createNew(UsageEnvironment& env,
        Boolean reuseFirstSource,
		CubemapFace* face) {
	return new H264VideoOnDemandServerMediaSubsession(env, reuseFirstSource, face);
}

H264VideoOnDemandServerMediaSubsession::H264VideoOnDemandServerMediaSubsession(UsageEnvironment& env,
	Boolean reuseFirstSource,
	CubemapFace* face)
: OnDemandServerMediaSubsession(env, reuseFirstSource),
fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL), face(face) {
  envi = &env;
}

H264VideoOnDemandServerMediaSubsession::~H264VideoOnDemandServerMediaSubsession() {
  delete[] fAuxSDPLine;
}

FramedSource* H264VideoOnDemandServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  estBitrate = 4000; // kbps, estimate

  //RandomFramedSource* source = RandomFramedSource::createNew(*envi, name);

  CubemapFaceSource* source = CubemapFaceSource::createNew(*envi, face);

  if (source == NULL) return NULL;

  // Create a framer for the Video Elementary Stream:
  return H264VideoStreamDiscreteFramer::createNew(envir(), source);
}

RTPSink* H264VideoOnDemandServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
        unsigned char rtpPayloadTypeIfDynamic,
        FramedSource* /*inputSource*/) {
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

static void afterPlayingDummy(void* clientData) {
  H264VideoOnDemandServerMediaSubsession* subsess = (H264VideoOnDemandServerMediaSubsession*) clientData;
  subsess->afterPlayingDummy1();
}

void H264VideoOnDemandServerMediaSubsession::afterPlayingDummy1() {
  // Unschedule any pending 'checking' task:
  envir().taskScheduler().unscheduleDelayedTask(nextTask());
  // Signal the event loop that we're done:
  setDoneFlag();
}
