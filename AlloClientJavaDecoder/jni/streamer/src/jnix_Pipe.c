//#include "jnix_Pipe.h"

#include "jnix.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

void Java_com_mathieu_alloclient_Pipe_open(JNIEnv *env, jobject obj)
{
	int fd[2];
	if(pipe(fd) != 0)
	{
		const char *message;
		switch(errno)
		{
		case EMFILE :
			message = "Too many file descriptors are in use by the process";
			break;
		case EFAULT :
			message = "File descriptor array is not valid";
			break;
		case ENFILE :
			message = "The system limit on the total number of open files has been reached";
			break;
		default:
			message = "An unknown exception occurred executing 'pipe(int fd[])'";
		}
		throwException(env, "java/io/IOException", message);
		return;
	}

	FileDescriptor in = newFileDescriptor(env, fd[0]);
	if(in == NULL)
		return;
	FileDescriptor out = newFileDescriptor(env, fd[1]);
	if(out == NULL)
		return;
	if(!setObjectField(env, obj, "input", "Ljava/io/FileDescriptor;", in))
		return;
	if(!setObjectField(env, obj, "output", "Ljava/io/FileDescriptor;", out))
		return;
}

void Java_com_mathieu_alloclient_Pipe_closeOutput(JNIEnv *env, jobject obj)
{
	FileDescriptor output;
	if(!getObjectField(env, obj, "output", "Ljava/io/FileDescriptor;", &output))
		return;
	if(!closeFileDescriptor(env, output))
		return;
}

void Java_com_mathieu_alloclient_Pipe_closeInput(JNIEnv *env, jobject obj)
{
	FileDescriptor input;
	if(!getObjectField(env, obj, "input", "Ljava/io/FileDescriptor;", &input))
		return;
	if(!closeFileDescriptor(env, input))
		return;
}
