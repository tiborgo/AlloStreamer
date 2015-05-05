set(SOURCES
	AlloPlayer.cpp
	H264WindowSink.cpp
	H264RawPixelsSink.cpp
	shader.cpp
	texture.cpp
	tutorial05.cpp
)
	
set(HEADERS
	H264WindowSink.h
	H264RawPixelsSink.h
	shader.hpp
	texture.hpp
)

set(RESOURCES
	TextureFragmentShader.fragmentshader
	TransformVertexShader.vertexshader
	uvtemplate.DDS
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
FIND_PACKAGE(OPENGL REQUIRED)

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
)
target_link_libraries(AlloPlayer
	${Boost_LIBRARIES}
	${FFMPEG_LIBRARIES}
	${Live555_LIBRARIES}
	${X264_LIBRARIES}
	${GLEW_LIBRARY}
	${GLFW_LIBRARY}
	${OPENGL_gl_LIBRARY}
	ws2_32
	winmm
)
target_compile_definitions(AlloPlayer
	PRIVATE
	AlloPlayer_RESOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(AlloPlayer
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}"
)