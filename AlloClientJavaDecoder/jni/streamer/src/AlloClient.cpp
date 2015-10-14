

#include "AlloClient.h"
extern "C"{
//#include<libavformat/avformat.h>
//#include<libavutil/avutil.h>
//#include<libavcodec/avcodec.h>
//#include<libavformat/avio.h>
//#include<libswscale/swscale.h>

}
#include"shared.h"
#include <android/log.h>

#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "AlloClient", __VA_ARGS__);
//using namespace android;

//AVFormatContext *mDataSource;
//AVCodecContext *mVideoTrack;
//AVBitStreamFilterContext *mConverter;
ConcurrentQueue<DecoderData> m_queue;

pthread_t decode_thread;
int mVideoIndex;

//void startStream(const char *rtspServer);

//status_t readFrame(u_int8_t* &buffer, int &frameSize, int64_t &presentationTime)
//{
//
//	AVPacket packet;
//
//	status_t ret;
//
//	ret = av_read_frame(mDataSource, &packet);
//
//	//printf("Packet size: %i, packet data: %u, packet pts: %llu", packet.size, packet.data[6], packet.pts);
//	//printf("nbstreams: %u", mDataSource->nb_streams);
//	if (packet.stream_index == mVideoIndex && ret == 0)
//	{
//
//		printf("Packet size before: %i", packet.size);
//		if (mConverter)
//		{
//			av_bitstream_filter_filter(mConverter, mVideoTrack, NULL, &packet.data, &packet.size, packet.data, packet.size, packet.flags & AV_PKT_FLAG_KEY);
//		}
//		//printf("Packet size after: %i", packet.size);
//		buffer = new u_int8_t[packet.size];
//		memcpy(buffer, packet.data, packet.size);
//
//		//printf("buffer0: %u", buffer[4]);
//		presentationTime = packet.pts;
//		frameSize = packet.size;
//
//		//printf("Packet size: %i", frameSize);
//		//(*buffer)->set_range(0, packet.size);
//		//(*buffer)->meta_data()->clear();
//		//(*buffer)->meta_data()->setInt32(kKeyIsSyncFrame, packet.flags & AV_PKT_FLAG_KEY);
//		//(*buffer)->meta_data()->setInt64(kKeyTime, packet.pts);
//
//
//	}
////	else if(ret<0)
////	{
////		avformat_open_input(&mDataSource, "rtsp://192.168.1.177:8554/h264ESVideoTest",NULL,NULL);
////	}
//	av_free_packet(&packet);
//
//	return ret;
//}

jmethodID mid;
jbyteArray jbArray;
JNIEnv *jniEnv = NULL;
jobject mainActivity = NULL;
_JavaVM* myJavaVM;
//jobject byteBuffer;
jbyteArray byteArray=NULL;
u_int8_t* buffer;
const int BUFFER_SIZE = 8000000;
jclass nalUnitClass, nalUnitClass1;
jmethodID constructor;
jmethodID setBuffer;
jobject frame;
int counter =0;
jboolean Java_com_mathieu_alloclient_javadecoder_NativeLib_decodeFrame(JNIEnv *env, jobject obj)
{
	printf("decode frame");

	//callback_handler();


	int frameSize=0;
	int64_t presentationTime=0;
	int err;

	/*
	 * Retrieve next frame with FFMPEG
	 */
//	err = readFrame(buffer, frameSize, presentationTime); //Read frame with FFMPEG



	/*
	 * Retrieve next frame with Live 555
	 */
		DecoderData d;
		int size = m_queue.get_size();
		printf("waiting for queue to be filled with data queue size: %i", size);
		timeval pauseBegin;
		gettimeofday(&pauseBegin, NULL);
		m_queue.wait_and_pop(d);

		timeval pauseEnd;
		gettimeofday(&pauseEnd, NULL);

		//printf("%i Time to get NAL off queue: %lu", counter,pauseEnd.tv_usec - pauseBegin.tv_usec);
		counter++;

		buffer = d.packetData;
		frameSize = d.packetSize;
		presentationTime = d.PTS.tv_usec;



		//printf("frameSize: %i, pts: %lu", frameSize, presentationTime);
		if(err==0){
			//printf("OK!");
		}
		else{
			//printf("Error or EOS");
		}

		//printf("frameSize: %i, framePTS: %lu", frameSize, presentationTime);
		if(frameSize > 0 /*&& err == 0*/)
		{
			//Make the buffer a ByteBuffer
//			jobject byteBuffer = env->NewDirectByteBuffer(buffer, frameSize);


			//Construct the NalUnit object from NDK
//			jclass nalUnitClass = env->FindClass("com/mathieu/alloclient/javadecoder/NalUnit");
//			jmethodID constructor = env->GetMethodID(nalUnitClass, "<init>", "(Ljava/nio/ByteBuffer;IJ)V");

			if(byteArray==NULL)
			{
				printf("initializing byte array");
				jbyteArray tmpByteArray = env->NewByteArray(BUFFER_SIZE);
				byteArray = (jbyteArray)env->NewGlobalRef(tmpByteArray);

				jclass tmpNalUnitClass1 = env->FindClass("com/mathieu/alloclient/javadecoder/NalUnit");
				nalUnitClass1 = (jclass)env->NewGlobalRef(tmpNalUnitClass1);

				setBuffer = env->GetMethodID(nalUnitClass1, "setBuffer", "([BIJ)V");
				//setBuffer = (jmethodID)env->NewGlobalRef(tmpSetBuffer);
			}
			printf("putting frame in buffer")
			//Set ByteBuffer pointer, ByteBuffer size and PTS in NAL object
			//printf("setting buffer");
			void *temp = env->GetPrimitiveArrayCritical((jarray)byteArray, 0);
			//printf("setting buffer1");
			memcpy(temp, buffer, frameSize);
			env->CallVoidMethod(frame, setBuffer, byteArray, frameSize, presentationTime);
			//printf("setting buffer2");
			env->ReleasePrimitiveArrayCritical(byteArray, temp, 0);
			//printf("set buffer");
			//env->DeleteGlobalRef(byteArray);
			//return frame;
			delete[] d.packetData;
			return true;
		}
		printf("frameSize less than 0");

	return false;
}

jobject Java_com_mathieu_alloclient_javadecoder_NativeLib_constructNalObject(JNIEnv *env, jobject obj)
{
	buffer= new u_int8_t[BUFFER_SIZE];

	//byteBuffer = env->NewDirectByteBuffer(buffer, BUFFER_SIZE);
//	byteArray = env->NewByteArray(BUFFER_SIZE);

	nalUnitClass = env->FindClass("com/mathieu/alloclient/javadecoder/NalUnit");
	//constructor = env->GetMethodID(nalUnitClass, "<init>", "(Ljava/nio/ByteBuffer;IJ)V");
	//frame = env->NewObject(nalUnitClass, constructor, byteBuffer, (jint)frameSize, (jlong)presentationTime);
	constructor = env->GetMethodID(nalUnitClass, "<init>", "(I)V");
	//setBuffer = env->GetMethodID(nalUnitClass, "setBuffer", "([BIJ)V");
	jobject tmp = env->NewObject(nalUnitClass, constructor, (jint)BUFFER_SIZE);
	frame = (jobject)env->NewGlobalRef(tmp);

	return frame;
}

u_int8_t* spspps = NULL;
int spsppssize = 0;
jobject Java_com_mathieu_alloclient_javadecoder_NativeLib_getSPSPPS(JNIEnv *env, jobject obj)
{
	//Make the buffer a ByteBuffer
	u_int8_t* buffer;
	int frameSize;
	int64_t presentationTime = 0;

	buffer = spspps;
	frameSize = spsppssize;

	printf("size: %i", frameSize);
	for(int i=0; i<frameSize; i++)
	{
		printf("%i", buffer[i]);
	}
	jobject byteBuffer = env->NewDirectByteBuffer(buffer, frameSize);

	//Construct the NalUnit object from NDK
	jobject csd = NULL;
	jclass nalUnitClass = env->FindClass("com/mathieu/alloclient/javadecoder/NalUnit");
	jmethodID constructor = env->GetMethodID(nalUnitClass, "<init>", "(Ljava/nio/ByteBuffer;IJ)V");
	csd = env->NewObject(nalUnitClass, constructor, byteBuffer, (jint)frameSize, (jlong)presentationTime);

	return csd;
}

bool Java_com_mathieu_alloclient_javadecoder_NativeLib_csdAvailable(JNIEnv *env, jobject obj)
{
	if (spsppssize<=0)
		return false;
	else
		return true;
}

jboolean Java_com_mathieu_alloclient_javadecoder_NativeLib_init(JNIEnv *env, jobject obj, jstring rtspServer)
{


	const char * server;
	server = env->GetStringUTFChars(rtspServer , NULL);

	printf("server is: %s", server);

	/*
	 * Start Live 555 server (do this from java code on separate java thread)
	 */
	//startStream(server);

	printf("Started server");
	/*
	 * Open the H264/RTSP source
	 * FFmpeg only
	 */
//	av_register_all();
//	avformat_network_init();
//
//	mDataSource = avformat_alloc_context();
//
//	int ret = avformat_open_input(&mDataSource, server ,NULL,NULL);
//
//	if(ret<0)
//	{
//		printf("Error connecting to server!")
//		return false;
//	}
//	else{
//		printf("connected!")
//	}
//
//
//	for (int i = 0; i < mDataSource->nb_streams; i++) {
//		if (mDataSource->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
//		  mVideoIndex = i;
//		  break;
//		}
//	}
//
//	mVideoTrack = mDataSource->streams[mVideoIndex]->codec;
//
//	printf("MVideoCodecID: %i  and H264 ID: %i", mVideoTrack->codec_id, CODEC_ID_H264);
////	printf("Pix format: %i", mVideoTrack->pix_fmt);
//
//
//    mConverter = av_bitstream_filter_init("h264_mp4toannexb");
//printf("Converted");
//    if (mVideoTrack->extradata[0] == 1) {
////    	for(int i=0; i<mVideoTrack->extradata_size; i++)
////    	{
////    		printf("extra data: %u", mVideoTrack->extradata[i])
////    	}
//    	//Send extra data (sps/pps) here?
//    	spspps = mVideoTrack->extradata;
//    	spsppssize = mVideoTrack->extradata_size;
//    }




	return true;
}


