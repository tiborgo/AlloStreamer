set(SOURCES
	CubemapExtractionPlugin.cpp
#PreviewWindow.cpp
	CubemapFaceD3D9.inl
	CubemapFaceD3D11.inl
    wxPreviewWindow.cpp
)

set(HEADERS
	CubemapExtractionPlugin.h
    PreviewWindow.h
	UnityPluginInterface.h
	CubemapFaceD3D9.h
	CubemapFaceD3D11.h
)

# include boost
find_package(Boost
  1.57                  # Minimum version
  REQUIRED              # Fail with error if Boost is not found
  COMPONENTS thread date_time system chrono   # Boost libraries by their canonical name
)                     # e.g. "date_time" for "libboost_date_time"
 
find_package(FFmpeg REQUIRED)
find_package(wxWidgets REQUIRED net gl core base)
  
add_library(CubemapExtractionPlugin
	SHARED
	${SOURCES}
	${HEADERS}
)
target_include_directories(CubemapExtractionPlugin
	PRIVATE
	${Boost_INCLUDE_DIRS}
	${FFMPEG_INCLUDE_DIRS}
    ${wxWidgets_INCLUDE_DIRS}
)
target_link_libraries(CubemapExtractionPlugin
	${Boost_LIBRARIES}
    ${wxWidgets_LIBRARIES}
    AlloShared
)
target_compile_definitions(CubemapExtractionPlugin
    PRIVATE
    ${wxWidgets_DEFINITIONS}
)
set_target_properties(CubemapExtractionPlugin
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}" # for Windows
    LIBRARY_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}/" # for OS X
)

if(WIN32)
    target_link_libraries(CubemapExtractionPlugin
        d3d9.lib # DirectX support
    )
endif()

if(APPLE)
    # trick unity into accepting dylibs as native plugins
    # from http://answers.unity3d.com/questions/23615/how-to-make-unity-find-dylib-files.html?sort=oldest
    set_target_properties(CubemapExtractionPlugin
        PROPERTIES
        SUFFIX ".bundle"
        PREFIX ""
    )
endif()