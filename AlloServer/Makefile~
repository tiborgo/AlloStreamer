deps= ../deps

LIBS= $(deps)/boost_1_54/build/lib/libboost_thread.a \
	$(deps)/boost_1_54/build/lib/libboost_system.a \
	-L$(deps)/live/liveMedia \
	-L$(deps)/live/groupsock \
	-L$(deps)/live/BasicUsageEnvironment \
	-L$(deps)/live/UsageEnvironment \
	-L$(deps)/install/lib \
	-lx264 \
	-lavcodec \
	-lavdevice \
	-lavfilter \
	-lavformat \
	-lavutil \
	-lswscale \
	-lswresample \
	-lliveMedia \
	-lgroupsock \
	-lBasicUsageEnvironment \
	-lUsageEnvironment \
	-lpthread \
	-lm \
	-lrt \

SRC= MyDeviceSource.cpp \
	AlloServer.c \
	serverUni.cpp \
	H264VideoOnDemandServerMediaSubsession.cpp \

INCLUDE= -I$(deps)/live/liveMedia/include \
	-I$(deps)/live/groupsock/include \
	-I$(deps)/live/UsageEnvironment/include \
	-I$(deps)/live/BasicUsageEnvironment/include \
	-I$(deps)/boost_1_54/build/include \
	-I$(deps)/install/include \


all:
	g++ -o AlloServer -g -Wl,--no-as-needed $(SRC) $(LIBS) $(INCLUDE) 
