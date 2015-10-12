deps= /usr/local/include

LIBS=  /usr/local/Cellar/boost/1.58.0/lib/libboost_system.dylib \
	/usr/local/Cellar/boost/1.58.0/lib/libboost_thread-mt.dylib \
	-L/usr/local/lib \
	-L/opt/local/lib \
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

SRC= MyDeviceSource.cpp \
	AlloServer.c \
	serverUni.cpp \
	H264VideoOnDemandServerMediaSubsession.cpp \

INCLUDE= -I$(deps)/liveMedia \
	-I$(deps)/groupsock \
	-I$(deps)/UsageEnvironment \
	-I$(deps)/BasicUsageEnvironment \
	-I/opt/local/include \
	-I/usr/local/include \
	-I/usr/local/Cellar/ffmpeg/2.6.2/include/ 


all:
	g++ -o AlloServer -g $(SRC) $(LIBS) $(INCLUDE) 
