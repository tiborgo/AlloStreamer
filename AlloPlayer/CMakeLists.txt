set(SOURCES
	AlloPlayer.cpp
	#H264WindowSink.cpp
	H264RawPixelsSink.cpp
	CubemapPreviewWindow.cpp
    StaticCubemapBackgroundApp.cpp
    DynamicCubemapBackgroundApp.cpp
    main.cpp
)
	
set(HEADERS
	#H264WindowSink.h
	H264RawPixelsSink.h
	CubemapPreviewWindow.h
    H264StreamCubemap.h
    DynamicCubemapBackgroundApp.hpp
    shader.h
    camera.h
)

set(RESOURCES
	skybox/back.jpg
    skybox/bottom.jpg
    skybox/front.jpg
    skybox/left.jpg
    skybox/right.jpg
    skybox/top.jpg
    skybox_small/back.jpg
    skybox_small/bottom.jpg
    skybox_small/front.jpg
    skybox_small/left.jpg
    skybox_small/right.jpg
    skybox_small/top.jpg
)


# include Boost, FFMpeg, live555, x264
find_package(Boost
  1.57                  # Minimum version
  REQUIRED              # Fail with error if Boost is not found
  COMPONENTS thread date_time system chrono   # Boost libraries by their canonical name
)                     # e.g. "date_time" for "libboost_date_time"
find_package(FFmpeg REQUIRED)
find_package(Live555 REQUIRED)
find_package(X264 REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)
FIND_PACKAGE(GLFW REQUIRED)
FIND_PACKAGE(GLM REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(SOIL REQUIRED)
find_package(Allocore REQUIRED)
find_package(Alloutil REQUIRED)

# packages for allocore
find_package(GLUT REQUIRED)
find_package(APR REQUIRED)
find_package(Portaudio REQUIRED)
find_package(Jsoncpp REQUIRED)
find_package(FreeImage REQUIRED)

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
	${GLEW_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
	${SOIL_INCLUDE_DIR}
        ${ALLOCORE_INCLUDE_DIR}
        ${ALLOUTIL_INCLUDE_DIR}
)
target_link_libraries(AlloPlayer
	${Boost_LIBRARIES}
	${FFMPEG_LIBRARIES}
	${Live555_LIBRARIES}
	${X264_LIBRARIES}
	${GLEW_LIBRARY}
	${GLFW_LIBRARY}
	${OPENGL_gl_LIBRARY}
	${SOIL_LIBRARY}
        ${ALLOUTIL_LIBRARY}
        ${ALLOCORE_LIBRARY}
        ${GLUT_glut_LIBRARY}
        ${APR_LIBRARY}
        ${PORTAUDIO_LIBRARY}
        ${Jsoncpp_LIBRARY}
        ${FREEIMAGE_LIBRARY}
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
