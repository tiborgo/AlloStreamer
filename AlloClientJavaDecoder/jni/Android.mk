#LOCAL_PATH := $(call my-dir)
#MY_BASE_JNI_PATH := $(CURDIR)/jni
#MY_BASE_JNI_PATH := $(CURDIR)/${NDK_APP_PROJECT_PATH}/jni
#include $(call all-subdir-makefiles)

MY_BASE_JNI_PATH := ${NDK_APP_PROJECT_PATH}/jni
#$(warning $(MY_BASE_JNI_PATH))
include $(call all-subdir-makefiles)
#$(warning $(CURDIR))

##stagefright
#include $(CLEAR_VARS)
##includes for all stagefright-related modules
#LOCAL_EXPORT_C_INCLUDES := ../../../../../Volumes/AndroidBuild/WORKING_DIRECTORY/frameworks/av/include
#LOCAL_EXPORT_C_INCLUDES += ../../../../../Volumes/AndroidBuild/WORKING_DIRECTORY/frameworks/native/include
#LOCAL_EXPORT_C_INCLUDES += ../../../../../Volumes/AndroidBuild/WORKING_DIRECTORY/system/core/include
#LOCAL_EXPORT_C_INCLUDES += ../../../../../Volumes/AndroidBuild/WORKING_DIRECTORY/hardware/libhardware/include
#LOCAL_EXPORT_C_INCLUDES += ../../../../../Volumes/AndroidBuild/WORKING_DIRECTORY/frameworks/native/include/media/openmax
#
#
##LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include/
##LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../Documents/workspace/AlloClient/jni/include
#
##$(warning $(LOCAL_C_INCLUDES))
#
##LOCAL_C_INCLUDES := $(SYSROOT)/Volumes/AndroidBuild/WORKING_DIRECTORY/frameworks/av/include
##LOCAL_C_INCLUDES += $(SYSROOT)/Volumes/AndroidBuild/WORKING_DIRECTORY/frameworks/native/include
#LOCAL_MODULE := stagefright
#LOCAL_SRC_FILES := libstagefright.so
#include $(PREBUILT_SHARED_LIBRARY)
#
##utils
#include $(CLEAR_VARS)
#LOCAL_MODULE := utils
#LOCAL_SRC_FILES := libutils.so
#include $(PREBUILT_SHARED_LIBRARY)
#
##media
#include $(CLEAR_VARS)
#LOCAL_MODULE := media
#LOCAL_SRC_FILES := libmedia.so
#include $(PREBUILT_SHARED_LIBRARY)
#
##binder
#include $(CLEAR_VARS)
#LOCAL_MODULE := binder
#LOCAL_SRC_FILES := libbinder.so
#include $(PREBUILT_SHARED_LIBRARY)
#
##AlloClient
#include $(CLEAR_VARS)
#LOCAL_MODULE    := AlloClient
#LOCAL_SRC_FILES := AlloClient.cpp
#LOCAL_SHARED_LIBRARIES := stagefright utils media binder streamer
##LOCAL_STATIC_LIBRARIES	:= \
##	BasicUsageEnvironment groupsock liveMedia UsageEnvironment 
#LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -landroid
#include $(BUILD_SHARED_LIBRARY)


