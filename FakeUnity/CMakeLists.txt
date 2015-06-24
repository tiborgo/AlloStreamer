set(SOURCES
	main.cpp
)

set(HEADERS
)

# include boost
find_package(Boost
  1.54                  # Minimum version
  REQUIRED              # Fail with error if Boost is not found
  COMPONENTS thread date_time system chrono   # Boost libraries by their canonical name
)                     # e.g. "date_time" for "libboost_date_time"
find_package(FFmpeg REQUIRED)

add_executable(FakeUnity
	${SOURCES}
	${HEADERS}
)
target_include_directories(FakeUnity
	PRIVATE
	${Boost_INCLUDE_DIRS}
	${FFMPEG_INCLUDE_DIRS}
)
target_link_libraries(FakeUnity
	${Boost_LIBRARIES}
    	AlloShared
)
set_target_properties(FakeUnity
    	PROPERTIES
    	RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE}"
)

# link against posix extension library on linux
if(CMAKE_SYSTEM MATCHES "Linux")
	target_link_libraries(FakeUnity
        	rt
		pthread
	)
endif()
