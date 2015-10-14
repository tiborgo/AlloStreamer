#include "jnix.h"

#include <errno.h>
#include <unistd.h>

jboolean setObjectField(JNIEnv *env, jobject obj, const char *fieldName, const char *fieldDescriptor, jobject value)
{
	jclass clazz = (*env)->GetObjectClass(env, obj);
	if(clazz == NULL)
		return JNI_FALSE;
	jfieldID field = (*env)->GetFieldID(env, clazz, fieldName, fieldDescriptor);
	(*env)->DeleteLocalRef(env, clazz);
	if(field == NULL)
		return JNI_FALSE;
	(*env)->SetObjectField(env, obj, field, value);
	return JNI_TRUE;
}

jboolean getObjectField(JNIEnv *env, jobject obj, const char *fieldName, const char *fieldDescriptor, jobject *out)
{
	jclass clazz = (*env)->GetObjectClass(env, obj);
	if(clazz == NULL)
		return JNI_FALSE;
	jfieldID fieldID = (*env)->GetFieldID(env, clazz, fieldName, fieldDescriptor);
	(*env)->DeleteLocalRef(env, clazz);
	if(fieldID == NULL)
		return JNI_FALSE;
	*out = (*env)->GetObjectField(env, obj, fieldID);
	return JNI_TRUE;
}

jboolean setIntField(JNIEnv *env, jobject obj, const char *fieldName, jint value)
{
	jclass clazz = (*env)->GetObjectClass(env, obj);
	if(clazz == NULL)
		return JNI_FALSE;
	jfieldID fieldID = (*env)->GetFieldID(env, clazz, fieldName, "I");
	(*env)->DeleteLocalRef(env, clazz);
	if(fieldID == NULL)
		return JNI_FALSE;
	(*env)->SetIntField(env, obj, fieldID, value);
	return JNI_TRUE;
}

jboolean getIntField(JNIEnv *env, jobject obj, const char *fieldName, jint *out)
{
	jclass clazz = (*env)->GetObjectClass(env, obj);
	if(clazz == NULL)
		return JNI_FALSE;
	jfieldID fieldID = (*env)->GetFieldID(env, clazz, fieldName, "I");
	(*env)->DeleteLocalRef(env, clazz);
	if(fieldID == NULL)
		return JNI_FALSE;
	*out = (*env)->GetIntField(env, obj, fieldID);
	return JNI_TRUE;
}

void throwException(JNIEnv *env, const char *name, const char *message)
{
	jclass clazz = (*env)->FindClass(env, name);
	if(clazz == NULL)
		return;
	(*env)->ThrowNew(env, clazz, message);
	(*env)->DeleteLocalRef(env, clazz);
}

FileDescriptor newFileDescriptor(JNIEnv *env, int descriptor)
{
	jclass clazz = (*env)->FindClass(env, "java/io/FileDescriptor");
	if(clazz == NULL)
		return NULL;
	jmethodID method = (*env)->GetMethodID(env, clazz, "<init>", "()V");
	if(method == NULL)
		return NULL;
	jobject fileDescriptor = (*env)->NewObject(env, clazz, method);
	if(fileDescriptor == NULL)
		return NULL;
	(*env)->DeleteLocalRef(env, clazz);
	if(!setIntField(env, fileDescriptor, FILE_DESCRIPTOR_PRIVATE_FIELD, descriptor))
		return NULL;
	return fileDescriptor;
}

jboolean closeFileDescriptor(JNIEnv *env, FileDescriptor fileDescriptor)
{
	jint fd;
	if(!getIntField(env, fileDescriptor, FILE_DESCRIPTOR_PRIVATE_FIELD, &fd))
		return JNI_FALSE;
	const char *message;
	switch(close(fd))
	{
	case EBADF :
		message = "fd isn't a valid open file descriptor";
		break;
	case EINTR :
		message = "The close() call was interrupted by a signal";
		break;
	case EIO :
		message = "An I/O error occurred";
		break;
	default: return JNI_TRUE;
	}
	throwException(env, "java/io/IOException", message);
	return JNI_FALSE;
}
