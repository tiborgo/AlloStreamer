set(SOURCES
    Renderer.cpp
 #   main.cpp
    main_2.cpp
)
	
set(HEADERS
    Renderer.hpp
    Win32_DirectXAppUtil.h
)

find_package(Boost
  1.54                  # Minimum version
  REQUIRED              # Fail with error if Boost is not found
  COMPONENTS thread date_time system chrono filesystem program_options regex # Boost libraries by their canonical name
)                     # e.g. "date_time" for "libboost_date_time"
#find_package(SDL REQUIRED)
find_package(FFmpeg REQUIRED)
find_package(OculusSDK REQUIRED)

add_executable(OculusPlayer
	${SOURCES}
	${HEADERS}
)
target_include_directories(OculusPlayer
	PRIVATE
	${Boost_INCLUDE_DIRS}
	${SDL_INCLUDE_DIR}
	${FFMPEG_INCLUDE_DIRS}
	${OCULUS_SDK_INCLUDE_DIRS}
)
target_link_libraries(OculusPlayer
	${Boost_LIBRARIES}
	${SDL_LIBRARY}
	${OCULUS_SDK_LIBRARIES}
    AlloReceiver
    winmm.lib 
    ws2_32.lib
)

set_target_properties(OculusPlayer
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE}"
)