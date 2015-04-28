set(SOURCES
	AlloPlayer.cpp
	H264WindowSink.cpp
)
	
set(HEADERS
	H264WindowSink.h
)

# include Boost, FFMpeg, live555, x264
#find_package(Boost
#  1.57                  # Minimum version
#  REQUIRED              # Fail with error if Boost is not found
#  COMPONENTS thread date_time system chrono   # Boost libraries by their canonical name
#)                     # e.g. "date_time" for "libboost_date_time"
#find_package(FFmpeg REQUIRED)
find_package(Live555 REQUIRED)
#find_package(X264 REQUIRED)

add_executable(AlloPlayer
	${SOURCES}
	${HEADERS}
)
target_include_directories(AlloPlayer
	PRIVATE
	#${Boost_INCLUDE_DIRS}
	${Live555_INCLUDE_DIRS}
	#${FFMPEG_INCLUDE_DIRS}
	#${X264_INCLUDE_DIRS}
)
target_link_libraries(AlloPlayer
	${Live555_LIBRARIES}
	ws2_32
	winmm
)

set_target_properties(AlloPlayer
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}"
)