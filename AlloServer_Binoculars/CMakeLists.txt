set(SOURCES
	MyDeviceSource.cpp
	AlloServer.cpp
	serverUni.cpp
	H264VideoOnDemandServerMediaSubsession.cpp
)
	
set(HEADERS
	ConcurrentQueue.h
	FrameData.h
	MyDeviceSource.hh
	H264VideoOnDemandServerMediaSubsession.hh
	shared.h
)

find_package(Boost
  1.54
  REQUIRED
  COMPONENTS system thread date_time chrono
)
find_package(FFmpeg REQUIRED)
find_package(Live555 REQUIRED)
find_package(X264 REQUIRED)

add_executable(AlloServer_Binoculars
	${SOURCES}
	${HEADERS}
)
target_include_directories(AlloServer_Binoculars
	PRIVATE
	${Boost_INCLUDE_DIRS}
	${Live555_INCLUDE_DIRS}
	${FFMPEG_INCLUDE_DIRS}
	${X264_INCLUDE_DIRS}
)
target_link_libraries(AlloServer_Binoculars
	${Boost_LIBRARIES}
	${FFMPEG_LIBRARIES}
	${Live555_LIBRARIES}
	${X264_LIBRARIES}
)
set_target_properties(AlloServer_Binoculars
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE}"
)

if(WIN32)
	target_link_libraries(AlloServer_Binoculars
        ws2_32
	)
endif()

# link against posix extension library on linux
if(CMAKE_SYSTEM MATCHES "Linux")
	target_link_libraries(AlloServer_Binoculars
        rt
	)
endif()
