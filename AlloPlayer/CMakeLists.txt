set(SOURCES
    DynamicCubemapBackgroundApp.cpp
    main.cpp
    Stats.cpp
)
	
set(HEADERS
    DynamicCubemapBackgroundApp.hpp
    Stats.h
)

set(RESOURCES
)


# packages for AlloPlayer
find_package(Boost
  1.54                  # Minimum version
  REQUIRED              # Fail with error if Boost is not found
  COMPONENTS thread date_time system chrono filesystem  # Boost libraries by their canonical name
)                     # e.g. "date_time" for "libboost_date_time"
find_package(FFmpeg REQUIRED)
find_package(Live555 REQUIRED)
find_package(X264 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Allocore REQUIRED)
find_package(Alloutil REQUIRED)

# packages for allocore
find_package(GLUT REQUIRED)
find_package(APR REQUIRED)
find_package(Portaudio REQUIRED)
find_package(Jsoncpp REQUIRED)
find_package(FreeImage REQUIRED)
find_package(GLEW REQUIRED)

add_executable(AlloPlayer
	${SOURCES}
	${HEADERS}
	${RESOURCES}
)
target_include_directories(AlloPlayer
	PRIVATE
	${Boost_INCLUDE_DIRS}
	${Live555_INCLUDE_DIRS}
	${FFMPEG_INCLUDE_DIRS}
	${X264_INCLUDE_DIRS}
	${SOIL_INCLUDE_DIR}
    ${ALLOCORE_INCLUDE_DIR}
    ${ALLOUTIL_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
)
target_link_libraries(AlloPlayer
	${Boost_LIBRARIES}
	${FFMPEG_LIBRARIES}
	${Live555_LIBRARIES}
	${X264_LIBRARIES}
	${OPENGL_gl_LIBRARY}
    ${ALLOUTIL_LIBRARY}
    ${ALLOCORE_LIBRARY}
    ${GLUT_glut_LIBRARY}
    ${APR_LIBRARY}
    ${PORTAUDIO_LIBRARY}
    ${Jsoncpp_LIBRARY}
    ${FREEIMAGE_LIBRARY}
	${GLEW_LIBRARY}
    AlloReceiver
)
target_compile_definitions(AlloPlayer
	PRIVATE
	AlloPlayer_RESOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/
)
set_target_properties(AlloPlayer
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Bin"
)

if(WIN32)
    target_link_libraries(AlloPlayer
        ws2_32
        winmm
    )
endif()
