LIBS= -framework CoreFoundation \
	-framework OpenGL \
	/usr/local/Cellar/boost/1.58.0/lib/libboost_system.dylib \
	/usr/local/Cellar/boost/1.58.0/lib/libboost_thread-mt.dylib \

INCLUDE= -I/Users/Mathieu/Development/boost_1_54_0 \
	-I/Users/tiborgoldschwendt/Desktop/AlloMathieu/AlloServer \
	-I/Users/tiborgoldschwendt/Desktop/AlloMathieu/UnityServerPlugin/RenderingPlugin/OSCHeaders \
	-I/Users/tiborgoldschwendt/Desktop/AlloMathieu/UnityServerPlugin/RenderingPlugin/OSCHeaders/ip \
	-I/Users/tiborgoldschwendt/Desktop/AlloMathieu/UnityServerPlugin/RenderingPlugin/OSCHeaders/osc \


SRC= IpEndpointName.cpp NetworkingUtils.cpp OscPrintReceivedElements.cpp OscTypes.cpp UdpSocket.cpp RenderingPlugin.cpp OSCReceivedElements.cpp
	

all:
	g++ -m64 -bundle $(INCLUDE) $(LIBS) $(SRC) -o UnityServerPlugin.bundle