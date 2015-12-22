set(SOURCES
	AlloServer.cpp
	H264NALUSource.cpp
	DiscreteFlowControlFilter.cpp
)
	
set(HEADERS
	config.h
	H264NALUSource.hpp
	AlloServer.h
	DiscreteFlowControlFilter.hpp
)

# include Boost, FFMpeg, live555, x264
find_package(Boost
  1.54                  # Minimum version
  REQUIRED              # Fail with error if Boost is not found
  COMPONENTS thread date_time system chrono filesystem program_options regex  # Boost libraries by their canonical name
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
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE}"
)

if(WIN32)
target_link_libraries(AlloServer
        ws2_32
)
endif()

# link against posix extension library on linux
if(CMAKE_SYSTEM MATCHES "Linux")
target_link_libraries(AlloServer
        rt
)
endif()
