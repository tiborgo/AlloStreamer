set(SOURCES
    Renderer.cpp
    main.cpp
)
	
set(HEADERS
    Renderer.hpp
)

find_package(Boost
  1.54                  # Minimum version
  REQUIRED              # Fail with error if Boost is not found
  COMPONENTS thread date_time system chrono filesystem program_options regex # Boost libraries by their canonical name
)                     # e.g. "date_time" for "libboost_date_time"
find_package(SDL REQUIRED)
find_package(FFmpeg REQUIRED)

add_executable(WindowedPlayer
	${SOURCES}
	${HEADERS}
)
target_include_directories(WindowedPlayer
	PRIVATE
	${Boost_INCLUDE_DIRS}
	${SDL_INCLUDE_DIR}
	${FFMPEG_INCLUDE_DIRS}
)
target_link_libraries(WindowedPlayer
	${Boost_LIBRARIES}
	${SDL_LIBRARY}
    AlloReceiver
)

set_target_properties(WindowedPlayer
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE}"
)