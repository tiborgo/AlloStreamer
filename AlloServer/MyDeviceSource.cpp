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
// NOTE: Sections of this code labeled "%%% TO BE WRITTEN %%%" are incomplete, and need to be written by the programmer
// (depending on the features of the particular device).
// Implementation

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#include "MyDeviceSource.hh"
#include <GroupsockHelper.hh> // for "gettimeofday()"
#include "shared.h"
//#include "stdafx.h"
#include <stdint.h>
#include <time.h>

int counter = 0;
int64_t start_sum = 0;
int64_t stop_sum = 0;
FILE * myfile;
/*
#define LOG_TIME(e) fprintf(myfile,"%i STAT:%s %.1f%% time, %.5fms/frame, %.1f%% calls\n",counter2, #e, overall ? 100.0f*m_Time[e]/overall : 0, m_Count[e] ? float(m_Time[e])/m_Count[e]/1000 : 0, count ? 100.0f*m_Count[e]/count : 0);

#define START_TIME(e) __int64 __##e = GetTimeInUSec();

#define STOP_TIME(e) m_Time[e] += (GetTimeInUSec() - __##e); ++m_Count[e];
*/

void logTimes(int64_t start, int64_t stop)
{
    double start_d = (double)start/counter*.001;
    double stop_d = (double)stop/counter*.001;

    //fprintf(myfile, "(milliseconds) start is: %.3lf, stop is: %.3lf, YUV/encoding took: %.3lf \n",start_d, stop_d, stop_d - start_d);
    //fprintf(myfile, "(microseconds) start is: %lld, stop is: %lld, YUV/encoding took: %lld \n",start, stop, stop - start);
    
    printf("(milliseconds) start is: %.3lf, stop is: %.3lf, YUV/encoding took: %.3lf \n",start_d, stop_d, stop_d - start_d);
    //printf("(microseconds) start is: %lld, stop is: %lld, YUV/encoding took: %lld \n",start, stop, stop - start);
    
    //fflush(myfile);
}
struct SwsContext *img_convert_ctx = NULL;

//int img_width = 960;
//int img_height = 540;
int rgb2yuv(AVFrame *frameRGB, AVFrame *frameYUV, AVCodecContext *c)
{
  char *err;
  if(img_convert_ctx == NULL) {
    int w = image_width;//c->width;
    int h = image_height; //c->height;
    img_convert_ctx = sws_getContext(w, h, PIX_FMT_RGB24,w, h,
                                     c->pix_fmt, SWS_BICUBIC,
                                     NULL, NULL, NULL);
    if(img_convert_ctx == NULL) {
      sprintf(err, "Cannot initialize the conversion context!\n");
      return -1;
    }
  }
    if(frameRGB->linesize[0] >0){
        frameRGB->data[0] += frameRGB->linesize[0]*(c->height -1);
  
        frameRGB->linesize[0] = -frameRGB->linesize[0];
    }
  return sws_scale(img_convert_ctx,frameRGB->data,
                      frameRGB->linesize , 0,c->height,
                      frameYUV->data, frameYUV->linesize );
}



MyDeviceSource*
MyDeviceSource::createNew(UsageEnvironment& env) {
  return new MyDeviceSource(env);
}

EventTriggerId MyDeviceSource::eventTriggerId = 0;

unsigned MyDeviceSource::referenceCount = 0;

struct timeval prevtime;
MyDeviceSource::MyDeviceSource(UsageEnvironment& env)
  : FramedSource(env){
  gettimeofday(&prevtime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
  if (referenceCount == 0) {
    // Any global initialization of the device would be done here:
    //%%% TO BE WRITTEN %%%
  }
  ++referenceCount;
myfile = fopen("/Users/tiborgoldschwendt/Desktop/AlloMathieu/Logs/deviceglxgears.log", "w");
  // Any instance-specific initialization of the device would be done here:
  //%%% TO BE WRITTEN %%%

  // We arrange here for our "deliverFrame" member function to be called
  // whenever the next frame of data becomes available from the device.
  //
  // If the device can be accessed as a readable socket, then one easy way to do this is using a call to
  //     envir().taskScheduler().turnOnBackgroundReadHandling( ... )
  // (See examples of this call in the "liveMedia" directory.)
  //
  // If, however, the device *cannot* be accessed as a readable socket, then instead we can implement it using 'event triggers':
  // Create an 'event trigger' for this device (if it hasn't already been done):
  if (eventTriggerId == 0) {
    eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
  }
      
    
}

MyDeviceSource::~MyDeviceSource() {
  // Any instance-specific 'destruction' (i.e., resetting) of the device would be done here:
  //%%% TO BE WRITTEN %%%
    counter++;
    
  --referenceCount;
  if (referenceCount == 0) {
    // Any global 'destruction' (i.e., resetting) of the device would be done here:
    //%%% TO BE WRITTEN %%%

    // Reclaim our 'event trigger'
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    eventTriggerId = 0;
  }
    fclose(myfile);
}
void MyDeviceSource::doGetNextFrame() {
    //fMaxSize = 200000;
  // This function is called (by our 'downstream' object) when it asks for new data.

  // Note: If, for some reason, the source device stops being readable (e.g., it gets closed), then you do the following:
  if (0 /* the source stops being readable */ /*%%% TO BE WRITTEN %%%*/) {
    handleClosure(this);
    return;
  }

  // If a new frame of data is immediately available to be delivered, then do this now:
  if (1 /* a new frame of data is immediately available to be delivered*/ /*%%% TO BE WRITTEN %%%*/) {
    deliverFrame();
  }

  // No new data is immediately available to be delivered.  We don't do anything more here.
  // Instead, our event trigger must be called (e.g., from a separate thread) when new data becomes available.
    
}

void MyDeviceSource::deliverFrame0(void* clientData) {
  ((MyDeviceSource*)clientData)->deliverFrame();
}
int rest = 0;
uint64_t globalPts = 0;
void MyDeviceSource::deliverFrame() {
  // This function is called when new frame data is available from the device.
  // We deliver this data by copying it to the 'downstream' object, using the following parameters (class members):
  // 'in' parameters (these should *not* be modified by this function):
  //     fTo: The frame data is copied to this address.
  //         (Note that the variable "fTo" is *not* modified.  Instead,
  //          the frame data is copied to the address pointed to by "fTo".)
  //     fMaxSize: This is the maximum number of bytes that can be copied
  //         (If the actual frame is larger than this, then it should
  //          be truncated, and "fNumTruncatedBytes" set accordingly.)
  // 'out' parameters (these are modified by this function):
  //     fFrameSize: Should be set to the delivered frame size (<= fMaxSize).
  //     fNumTruncatedBytes: Should be set iff the delivered frame would have been
  //         bigger than "fMaxSize", in which case it's set to the number of bytes
  //         that have been omitted.
  //     fPresentationTime: Should be set to the frame's presentation time
  //         (seconds, microseconds).  This time must be aligned with 'wall-clock time' - i.e., the time that you would get
  //         by calling "gettimeofday()".
  //     fDurationInMicroseconds: Should be set to the frame's duration, if known.
  //         If, however, the device is a 'live source' (e.g., encoded from a camera or microphone), then we probably don't need
  //         to set this variable, because - in this case - data will never arrive 'early'.
  // Note the code below.
  
    fprintf(myfile, "fMaxSize at beginning of function: %i \n", fMaxSize);
    fflush(myfile);
    
  counter++;
  int ret;
  int got_output = 0;
  if (!isCurrentlyAwaitingData()) return; // we're not ready for the data yet

  fNumTruncatedBytes = rest;
  if(fNumTruncatedBytes > 0){
    u_int8_t* newFrameDataStart = (u_int8_t*)pkt.data + pkt.size - fNumTruncatedBytes; //%%% TO BE WRITTEN %%%
    if(fNumTruncatedBytes >fMaxSize){
        fprintf(myfile, "truncatedBytes greater than MaxSize: $d\n", pkt.size);
        fflush(myfile);
      fFrameSize = fMaxSize;
      fNumTruncatedBytes -= fMaxSize;
    }
    else{
      fFrameSize = fNumTruncatedBytes;
      fNumTruncatedBytes = 0;
    }
    
    memmove(fTo, newFrameDataStart, fFrameSize);
    if(fNumTruncatedBytes == 0)
      av_free_packet(&pkt);
    rest = fNumTruncatedBytes;
    gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
    FramedSource::afterGetting(this);
    return;
  }

  //gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
    //uint64_t dt = av_gettime(); //(fPresentationTime.tv_sec - prevtime.tv_sec)* 1000000 + fPresentationTime.tv_usec - prevtime.tv_usec;

  globalPts+=100000;

  gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
  //int64_t start = fPresentationTime.tv_sec *1000000 +fPresentationTime.tv_usec;
  int64_t start = av_gettime();
//    fprintf(myfile, "Convert color \n");
//    fflush(myfile);
  //pthread_mutex_lock(&mutex);
  //rgb2yuv(iframe,frame,c);
  //pthread_mutex_unlock(&mutex);
  //mypts *= 1000000;
  //frame->pts+=start;
  /* encode the image */
  av_init_packet(&pkt);
  pkt.data = NULL;    // packet data will be allocated by the encoder
  pkt.size = 0;
//    fprintf(myfile, "Encode video \n");
//    fflush(myfile);
  ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
    
  fprintf(myfile, "packet size: %i \n", pkt.size);
  fflush(myfile);
    
  //int64_t stop = gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.

  
    
  if (ret < 0) {
    fprintf(stderr, "Error encoding frame\n");
    exit(1);
  }
//  if(got_output) printf("got output%d\n",pkt.size);
  u_int8_t* newFrameDataStart = (u_int8_t*)pkt.data; //%%% TO BE WRITTEN %%%
  unsigned newFrameSize = pkt.size; //%%% TO BE WRITTEN %%%

  // Deliver the data here:
  if (newFrameSize > fMaxSize) {
      fFrameSize = fMaxSize;
      rest = fNumTruncatedBytes = newFrameSize - fMaxSize;
      //printf("truncated\n");
      
      fprintf(myfile, "frameSize %i larger than maxSize %i\n", pkt.size, fMaxSize);
      fflush(myfile);
  } else {
    fFrameSize = newFrameSize;
    rest = fNumTruncatedBytes = 0;
      
  }

  
  // If the device is *not* a 'live source' (e.g., it comes instead from a file or buffer), then set "fDurationInMicroseconds" here.
  memmove(fTo, newFrameDataStart, fFrameSize);
  if(fNumTruncatedBytes == 0)
    av_free_packet(&pkt);
 // printf("got output%d %d\n",got_output,pkt.data);
  // After delivering the data, inform the reader that it is now available:
  FramedSource::afterGetting(this);
    
  int64_t stop = av_gettime();//fPresentationTime.tv_sec *1000000 +fPresentationTime.tv_usec;
  start_sum += start;
  stop_sum += stop;
//  
  if(counter%30==0)
  {
    printf("start stop: %lld %lld \n", start_sum, stop_sum);
    logTimes(start_sum, stop_sum);
  }
}


// The following code would be called to signal that a new frame of data has become available.
// This (unlike other "LIVE555 Streaming Media" library code) may be called from a separate thread.
// (Note, however, that "triggerEvent()" cannot be called with the same 'event trigger id' from different threads.
// Also, if you want to have multiple device threads, each one using a different 'event trigger id', then you will need
// to make "eventTriggerId" a non-static member variable of "DeviceSource".)
void signalNewFrameData() {
  TaskScheduler* ourScheduler = NULL; //%%% TO BE WRITTEN %%%
  MyDeviceSource* ourDevice  = NULL; //%%% TO BE WRITTEN %%%

  if (ourScheduler != NULL) { // sanity check
    ourScheduler->triggerEvent(MyDeviceSource::eventTriggerId, ourDevice);
  }
}
