/*
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * This is a port of the infamous "gears" demo to straight GLX (i.e. no GLUT)
 * Port by Brian Paul  23 March 2001

 * See usage() below for command line options.
 */

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#include "MyDeviceSource.hh"
void startRTSP();
void signalNewFrameData();
#include "FrameData.h"
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include "shared.h"
extern "C" {
#include <math.h>
#include <stdlib.h>

#include <string.h>

//#include <pthread.h>
//#define FORSTREAMING 1
//#ifdef FORSTREAMING
//#include <libavcodec/avcodec.h>
//#include <libavutil/opt.h>
//#include <libavutil/imgutils.h>
//#include <libavutil/time.h>
//#include <libswscale/swscale.h>
}

#include <chrono>
#include <thread>

/* disable printf */
//#define printf(...)

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//#include "shared.h"

extern MyDeviceSource *fileSource;

AVCodec *codec;
AVCodecContext *c= NULL;
AVFrame *iframe;
AVFrame *iframeScaled;
AVFrame *frame;
AVPacket pkt;

ConcurrentQueue<x264_nal_t> m_queue;
SwsContext* convertCtx = NULL;
SwsContext* scaleCtx = NULL;
x264_param_t param;
x264_t* encoder;
x264_picture_t pic_in, pic_out;
    
//Encoder param
//extern const int image_width = 1920;
//extern const int image_height = 1080;
//extern const int image_width = 960;
//extern const int image_height = 540;

extern const int image_width = 1024;
extern const int image_height = 576;

const int texture_width = 1024;
const int texture_height = 576;

//extern const int image_width = 1024;
//extern const int image_height = 576;

//extern const int image_width = 480;
//extern const int image_height = 270;
//extern const int image_width = 720;
//extern const int image_height = 400;
//extern const int image_width = 64;
//extern const int image_height = 64;
const int bit_rate = 15000000;
const char * preset_val = "ultrafast";
const char * tune_val = "zerolatency:fastdecode";
const int FPS = 60;

//unsigned char *image;
//unsigned char image[image_width*image_height*3];


//#include <sys/time.h>
//#include <unistd.h>


AVFrame *frame1;

using namespace boost::interprocess;
unsigned char randomPixels[/*image_width*image_height*/texture_height*texture_width*3];
unsigned char frame_bufferScaled[image_height*image_width*3];
unsigned char frame_bufferYUV[image_height*image_width*3];
mapped_region region;
FILE * logz;
static void initEncoder()
{

  int ret;
  //uint8_t endcode[] = { 0, 0, 1, 0xb7 };
  //printf("Encode video file %s\n", filename);
  /* find the mpeg1 video encoder */
  codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  if (!codec) {
    fprintf(stderr, "Codec not found\n");
    exit(1);
  }
  c = avcodec_alloc_context3(codec);
  if (!c) {
    fprintf(stderr, "Could not allocate video codec context\n");
    exit(1);
  }
  /* put sample parameters */
  c->bit_rate = bit_rate;
  /* resolution must be a multiple of two */
    c->width = image_width;
  c->height = image_height;
  /* frames per second */
  c->time_base= av_make_q(1,FPS);
  c->gop_size = 20; /* emit one intra frame every ten frames */
  c->max_b_frames=0;
  c->pix_fmt = AV_PIX_FMT_YUV420P;
 

  //if(codec_id == AV_CODEC_ID_H264){
    av_opt_set(c->priv_data, "preset", preset_val, 0);
    av_opt_set(c->priv_data, "tune", tune_val, 0);
    //av_opt_set(c->priv_data, "x264opts", x264_val, 0);
      
      
    //av_opt_set(c->priv_data, "profile", "high", 0);
    //av_opt_set(c->priv_data, "sliced-threads", "", 0);
    

  //}
  /* open it */
  if (avcodec_open2(c, codec, NULL) < 0) {
    fprintf(stderr, "Could not open codec\n");
    exit(1);
  }
  
  iframe = avcodec_alloc_frame();
  //iframeScaled = avcodec_alloc_frame();
  frame = avcodec_alloc_frame();
  
  if (!frame|| !iframe) {
    fprintf(stderr, "Could not allocate video frame\n");
    exit(1);
  }
    
    iframe->format = AV_PIX_FMT_RGB24;
    iframe->width  = texture_width;//image_width;//c->width;
    iframe->height = texture_height;//image_height;//c->height;
    
    //iframeScaled->format = AV_PIX_FMT_RGB24;
    //iframeScaled->width  = image_width;//image_width;//c->width;
    //iframeScaled->height = image_height;//image_height;//c->height;
    
    frame->format = AV_PIX_FMT_YUV420P;//c->pix_fmt;
    frame->width  = image_width;//c->width;
    frame->height = image_height;//c->height;
//  frame->time_base= (AVRational){1,10};


  /* the image can be allocated by any means and av_image_alloc() is
   * just the most convenient way if av_malloc() is to be used */
    ret = av_image_alloc(iframe->data, iframe->linesize, image_width, image_height, AV_PIX_FMT_RGB24, 32);
    //ret = av_image_alloc(iframeScaled->data, frame->linesize, image_width, image_height, AV_PIX_FMT_RGB24, 32);
    ret = av_image_alloc(frame->data, frame->linesize, image_width, image_height, AV_PIX_FMT_YUV420P, 32);
  
    
  if (ret < 0) {
    fprintf(stderr, "Could not allocate raw picture buffer\n");
    exit(1);
  }

  //scaleCtx = sws_getContext(texture_width, texture_height, PIX_FMT_RGB24, image_width, image_height, PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
  convertCtx = sws_getContext(image_width, image_height, PIX_FMT_RGB24, image_width, image_height, PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    

  
}

int num = 0;
void *addr;
void uploadFrames()
{
  
    while(sharedData->shutdownServer==false/*true*/)
    {
        avpicture_fill((AVPicture *) iframe, sharedData->pixels/*randomPixels*/, AV_PIX_FMT_RGB24,image_width,image_height);
        //avpicture_fill((AVPicture *) iframeScaled, frame_bufferScaled, AV_PIX_FMT_RGB24,image_width,image_height);
        //avpicture_fill((AVPicture *) frame, frame_bufferYUV, AV_PIX_FMT_YUV420P,image_width,image_height);

        //First scale the image (to avoid segfault)
        //sws_scale(scaleCtx, iframe->data, iframe->linesize,0, iframe->height, iframeScaled->data, iframeScaled->linesize);

        //std::cout<< iframe->linesize << " " << iframe->linesize[1] << " " << iframe->height << std::endl;

        //flip the image to convert to YUV420P
        if(iframe->linesize[0] > 0)
        {
            iframe->data[0] += iframe->linesize[0]*(image_height -1);
            iframe->linesize[0] = -iframe->linesize[0];
        }
        
        //Convert to YUV420P
        sws_scale(convertCtx, iframe->data, iframe->linesize,0, image_height, frame->data, frame->linesize);

        num++;
        if(num%15 == 0)
        {
            for(int i=0; i<image_height*image_width*3; i++)
            {
                randomPixels[i] = rand() % 255;
                
            }
        }
		std::this_thread::sleep_for(std::chrono::microseconds(15000));

    }
    fprintf(logz,"exiting... \n");
    fflush(logz);
    
}


int
main(int argc, char *argv[])
{
    logz = fopen(ROOT_DIR "/Logs/AlloServer.log", "w");


  avcodec_register_all();
  initEncoder();

  

  //Open already created shared memory object.
  shared_memory_object shm (open_only, "MySharedMemory2", read_write);
  
  fprintf(logz,"mapping region... \n");
  fflush(logz);
  
  //Map the whole shared memory in this process
  region = mapped_region(shm, read_write);
  
  //Check that memory was initialized to 1
  addr = region.get_address();
  fprintf(logz,"constructing object in memory \n");
  fflush(logz);
  
  sharedData = static_cast<FrameData*>(addr);

  startRTSP();

  fprintf(logz, "Uploading frames to libav... \n");
  fflush(logz);

 for(int i=0; i<texture_height*texture_width*3; i++)
{
    randomPixels[i] = rand() % 255;

} 



/*  frame1 = avcodec_alloc_frame();*/
/*  frame1->format = AV_PIX_FMT_YUV420P;*/
/*  frame1->width  = image_width;//c->width;*/
/*  frame1->height = image_height;//c->height;*/
/*  int ret2 = av_image_alloc(frame1->data, frame1->linesize, frame1->width, frame1->height, AV_PIX_FMT_YUV420P, 32);*/
  
  //avpicture_fill((AVPicture *) frame1, randomPixels/*sharedData->pixels*/, AV_PIX_FMT_RGB24,image_width,image_height);
  //avpicture_fill((AVPicture *) iframe, /*sharedData->pixels*/randomPixels, AV_PIX_FMT_RGB24,image_width,image_height);

  uploadFrames();//Does not return
  
  unsigned int winWidth = image_width, winHeight = image_height;
  int x = 0, y = 0;


  return 0;
}

