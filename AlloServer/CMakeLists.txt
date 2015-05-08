set(SOURCES
	config.cpp
	serverUni.cpp
	#RandomFramedSource.cpp
	H264VideoOnDemandServerMediaSubsession.cpp
	AlloServer.cpp
	CubemapFaceSource.cpp
)
	
set(HEADERS
	shared.h
	config.h
	#RandomFramedSource.h
	FrameData.h
	H264VideoOnDemandServerMediaSubsession.h
	concurrent_queue.h
	CubemapFaceSource.h
	AlloServer.h
)

# include Boost, FFMpeg, live555, x264
find_package(Boost
  1.57                  # Minimum version
  REQUIRED              # Fail with error if Boost is not found
  COMPONENTS thread date_time system chrono   # Boost libraries by their canonical name
)                     # e.g. "date_time" for "libboost_date_time"
find_package(FFmpeg REQUIRED)
find_package(Live555 REQUIRED)
find_package(X264 REQUIRED)

add_executable(AlloServer
	${SOURCES}
	${HEADERS}
)
target_include_directories(AlloServer
	PRIVATE
	${Boost_INCLUDE_DIRS}
	${Live555_INCLUDE_DIRS}
	${FFMPEG_INCLUDE_DIRS}
	${X264_INCLUDE_DIRS}
)
target_link_libraries(AlloServer
	${Boost_LIBRARIES}
	${FFMPEG_LIBRARIES}
	${Live555_LIBRARIES}
	${X264_LIBRARIES}
	AlloShared
)
set_target_properties(AlloServer
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Bin"
)

if(WIN32)
target_link_libraries(AlloPlayer
        ws2_32
)
endif()
