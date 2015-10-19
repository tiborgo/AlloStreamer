LOCAL_PATH := $(call my-dir)




#utils


#include $(CLEAR_VARS)
#LOCAL_MODULE := avcodec
#LOCAL_SRC_FILES := ffmpegAndroid/armv7/lib/libavcodec.so
#include $(PREBUILT_SHARED_LIBRARY)
#
#include $(CLEAR_VARS)
#LOCAL_MODULE := avformat
#LOCAL_SRC_FILES := ffmpegAndroid/armv7/lib/libavformat.so
#include $(PREBUILT_SHARED_LIBRARY)
#
#include $(CLEAR_VARS)
#LOCAL_MODULE := avutil
#LOCAL_SRC_FILES := ffmpegAndroid/armv7/lib/libavutil.so
#include $(PREBUILT_SHARED_LIBRARY)



include $(CLEAR_VARS)

MY_MODULE_DIR 			:= streamer

LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

LOCAL_MODULE    		:= $(MY_MODULE_DIR)
LOCAL_SRC_FILES 		:= \
	$(subst $(MY_BASE_JNI_PATH)/$(MY_MODULE_DIR)/,,$(wildcard $(MY_BASE_JNI_PATH)/$(MY_MODULE_DIR)/src/*.c*))
LOCAL_LDLIBS 			:= -lm -llog -lz -landroid -lGLESv2 -L$(SYSROOT)/usr/lib 
#$(warning $(subst $(MY_BASE_JNI_PATH)/$(MY_MODULE_DIR)/,,$(wildcard $(MY_BASE_JNI_PATH)/$(MY_MODULE_DIR)/src/*.c*)))
#$(warning $(MY_BASE_JNI_PATH))
#$(warning $(MY_MODULE_DIR))

#\
#	-L$(CURDIR)/${NDK_APP_PROJECT_PATH}/libs/$(TARGET_ABI_SUBDIR) -ljnix
LOCAL_C_INCLUDES 		:= \
	$(MY_BASE_JNI_PATH)/$(MY_MODULE_DIR)/jni-include \
	$(MY_BASE_JNI_PATH)/$(MY_MODULE_DIR)/include \
	$(MY_BASE_JNI_PATH)/BasicUsageEnvironment/include \
	$(MY_BASE_JNI_PATH)/groupsock/include \
	$(MY_BASE_JNI_PATH)/liveMedia/include \
	$(MY_BASE_JNI_PATH)/UsageEnvironment/include \
	$(MY_BASE_JNI_PATH)/$(MY_MODULE_DIR)/ffmpegAndroid/armv7/include \
	
LOCAL_CFLAGS			:= \
	-DFILE_DESCRIPTOR_PRIVATE_FIELD=\"descriptor\"

LOCAL_STATIC_LIBRARIES	:= \
	BasicUsageEnvironment groupsock liveMedia UsageEnvironment 


#ffmpeg avcodec avformat avutil
	
include $(BUILD_SHARED_LIBRARY)

#$(MY_BASE_JNI_PATH)/$(MY_MODULE_DIR)/ffmpeg/android/armv7-a/libffmpeg.so
#$(MY_BASE_JNI_PATH)/$(MY_MODULE_DIR)/ffmpeg/android/armv7-a/include \


##AlloClient
#include $(CLEAR_VARS)
#LOCAL_MODULE    := AlloClient
#LOCAL_SRC_FILES := src/AlloClient.cpp
#LOCAL_SHARED_LIBRARIES := stagefright utils media binder streamer
##LOCAL_STATIC_LIBRARIES	:= \
##	BasicUsageEnvironment groupsock liveMedia UsageEnvironment 
#LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -landroid
#include $(BUILD_SHARED_LIBRARY)