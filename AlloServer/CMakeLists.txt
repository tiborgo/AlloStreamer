cmake_minimum_required (VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../CMakeModules/")

project(AlloServer)

# Use unicode in project
add_definitions(-DUNICODE -D_UNICODE)


set(SOURCES
	config.cpp
	serverUni.cpp
	RandomFramedSource.cpp
	#MyDeviceSource.cpp
	H264VideoOnDemandServerMediaSubsession.cpp
	AlloServer.cpp)
	
set(HEADERS
	shared.h
	config.h
	#MyDeviceSource.h
	RandomFramedSource.h
	FrameData.h
	H264VideoOnDemandServerMediaSubsession.h
	ConcurrentQueue.h)

# include Boost
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_STATIC_LIBS OFF)
add_definitions(-DBOOST_ALL_DYN_LINK)
find_package(Boost
  1.57                  # Minimum version
  REQUIRED              # Fail with error if Boost is not found
  COMPONENTS thread date_time system chrono   # Boost libraries by their canonical name
  )                     # e.g. "date_time" for "libboost_date_time"

# include FFMPEG
find_package(FFmpeg REQUIRED)

# include Live555
find_package(Live555 REQUIRED)

find_package(X264 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Live555_INCLUDE_DIRS})
include_directories(${FFMPEG_INCLUDE_DIRS})
include_directories(${X264_INCLUDE_DIRS})
add_library(AlloServer SHARED ${SOURCES} ${HEADERS})
target_link_libraries(AlloServer ${Boost_LIBRARIES} Ws2_32.lib)
target_link_libraries(AlloServer ${Live555_LIBRARIES})
target_link_libraries(AlloServer ${FFMPEG_LIBRARIES})
target_link_libraries(AlloServer ${X264_LIBRARIES})

# set(UNITY_PROJECT_PLUGIN_DIR "../AlloStreamer/Assets/Plugins")

#set_target_properties(CubemapExtractionPlugin
#    PROPERTIES
#    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${UNITY_PROJECT_PLUGIN_DIR}"
#    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${UNITY_PROJECT_PLUGIN_DIR}"
#    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${UNITY_PROJECT_PLUGIN_DIR}"
#)