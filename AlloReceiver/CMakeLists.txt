set(SOURCES
	AlloReceiver.cpp
	H264RawPixelsSink.cpp
    Stats.cpp
    CubemapSource.cpp
    Source.cpp
    H264CubemapSource.cpp
    RTSPCubemapSourceClient.cpp
    RTSPCubemapSource.cpp
)

set(HEADERS
	H264RawPixelsSink.h
    Stats.h
    AlloReceiver.h
    CubemapSource.hpp
    Source.hpp
    H264CubemapSource.h
    RTSPCubemapSourceClient.hpp
    RTSPCubemapSource.hpp
)


# packages for AlloReceiver
find_package(Boost
  1.54                  # Minimum version
  REQUIRED              # Fail with error if Boost is not found
  COMPONENTS thread date_time system chrono filesystem  # Boost libraries by their canonical name
)                     # e.g. "date_time" for "libboost_date_time"
find_package(FFmpeg REQUIRED)
find_package(Live555 REQUIRED)
find_package(X264 REQUIRED)

add_library(AlloReceiver
	SHARED
	${SOURCES}
	${HEADERS}
)
target_include_directories(AlloReceiver
	PRIVATE
	${Boost_INCLUDE_DIRS}
	${Live555_INCLUDE_DIRS}
	${FFMPEG_INCLUDE_DIRS}
	${X264_INCLUDE_DIRS}
)
target_link_libraries(AlloReceiver
	${Boost_LIBRARIES}
	${FFMPEG_LIBRARIES}
	${Live555_LIBRARIES}
	${X264_LIBRARIES}
    AlloShared
)
target_compile_definitions(AlloReceiver
	PRIVATE
	AlloReceiver_RESOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/
)
set_target_properties(AlloReceiver
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Bin"
)

if(WIN32)
    target_link_libraries(AlloReceiver
        ws2_32
        winmm
    )
endif()
