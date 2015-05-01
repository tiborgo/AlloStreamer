set(SOURCES
	CubemapFace.cpp
	Signals.cpp
)
	
set(HEADERS
	CubemapFace.h
	Signals.h
)

find_package(Boost
  1.57                  # Minimum version
  REQUIRED
  signals
  date_time
  chrono
  system
)
find_package(FFmpeg REQUIRED)

add_library(AlloShared
	SHARED
	${SOURCES} ${HEADERS}
)
target_link_libraries(AlloShared
	${Boost_LIBRARIES}
)
target_include_directories(AlloShared
	PRIVATE
	${Boost_INCLUDE_DIRS}
	${FFMPEG_INCLUDE_DIRS}
)
set_target_properties(AlloShared
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}"
)