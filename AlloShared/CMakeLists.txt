set(SOURCES
	Cubemap.cpp
    Process.cpp
    Allocator.cpp
)
	
set(HEADERS
	Cubemap.hpp
    config.h
    concurrent_queue.h
    Process.h
    Allocator.h
)

find_package(Boost
  1.54                  # Minimum version
  REQUIRED
  signals
  date_time
  chrono
  system
  filesystem
)
find_package(FFmpeg REQUIRED)

add_library(AlloShared
	STATIC
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
	ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Lib/${CMAKE_BUILD_TYPE}"
)
