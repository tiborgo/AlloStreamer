set(SOURCES
	RenderingPlugin.cpp
)

set(HEADERS
	UnityPluginInterface.h
)

# include boost
find_package(Boost
  1.54
  REQUIRED
  COMPONENTS thread date_time system chrono
)
find_package(OpenGL REQUIRED)
find_package(OSCPack REQUIRED) 
 
add_library(RenderingPlugin
	SHARED
	${SOURCES}
	${HEADERS}
)
target_include_directories(RenderingPlugin
	PRIVATE
	${Boost_INCLUDE_DIRS}
	${OSCPack_INCLUDE_DIRS}
)
target_link_libraries(RenderingPlugin
	${Boost_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
	${OSCPack_LIBRARIES}
)
set_target_properties(RenderingPlugin
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}" # for Windows
    LIBRARY_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}/" # for OS X
)

if(WIN32)
	target_link_libraries(RenderingPlugin
        ws2_32
		winmm
	)
endif()

if(APPLE)
    # trick unity into accepting dylibs as native plugins
    # from http://answers.unity3d.com/questions/23615/how-to-make-unity-find-dylib-files.html?sort=oldest
    set_target_properties(RenderingPlugin
        PROPERTIES
        SUFFIX ".bundle"
        PREFIX ""
    )
endif()
