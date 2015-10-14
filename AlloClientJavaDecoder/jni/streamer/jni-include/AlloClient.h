//

//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>
#ifndef _Included_AlloClient
#define _Included_AlloClient
#include <jni.h>
#include <stdint.h>
#include <string.h>

//#include <media/stagefright/MediaBuffer.h>

	void* decode_frames(void*);


extern "C"
{
	jboolean Java_com_mathieu_alloclient_javadecoder_NativeLib_init(JNIEnv *env, jobject obj, jstring rtspServer);
	jboolean Java_com_mathieu_alloclient_javadecoder_NativeLib_decodeFrame(JNIEnv *env, jobject obj);
	jobject Java_com_mathieu_alloclient_javadecoder_NativeLib_constructNalObject(JNIEnv *env, jobject obj);
	jobject Java_com_mathieu_alloclient_javadecoder_NativeLib_getSPSPPS(JNIEnv *env, jobject obj);
	bool Java_com_mathieu_alloclient_javadecoder_NativeLib_csdAvailable(JNIEnv *env, jobject obj);
}
#endif
