set(SOURCES
	CubemapExtractionPlugin.cpp
	PreviewWindow.cpp
	CubemapFaceD3D9.cpp
	CubemapFaceD3D11.cpp
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
  
add_library(CubemapExtractionPlugin
	SHARED
	${SOURCES}
	${HEADERS})
target_include_directories(CubemapExtractionPlugin
	PRIVATE
	${Boost_INCLUDE_DIRS}
)
target_link_libraries(CubemapExtractionPlugin
	${Boost_LIBRARIES}
	d3d9.lib # DirectX support
	AlloShared
)

set_target_properties(CubemapExtractionPlugin
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${UNITY_PROJECT_PLUGIN_DIR}"
)