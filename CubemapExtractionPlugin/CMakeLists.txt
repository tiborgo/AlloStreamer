cmake_minimum_required (VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../../CMakeModules/")

project(CubemapExtractionPlugin)

# Use unicode in project
add_definitions(-DUNICODE)


set(SOUCRES
	CubemapExtractionPlugin.cpp
	PreviewWindow.cpp)
	
set(HEADERS
	CubemapExtractionPlugin.h
	PreviewWindow.h
	UnityPluginInterface.h)

# include boost
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_STATIC_LIBS OFF)
add_definitions(-DBOOST_ALL_DYN_LINK)
find_package(Boost
  1.57                  # Minimum version
  REQUIRED              # Fail with error if Boost is not found
  COMPONENTS thread date_time system chrono   # Boost libraries by their canonical name
  )                     # e.g. "date_time" for "libboost_date_time"

# include DirectX
# find_package(DirectX REQUIRED)
  
include_directories(${Boost_INCLUDE_DIRS})
add_library(CubemapExtractionPlugin SHARED ${SOUCRES} ${HEADERS})
target_link_libraries(CubemapExtractionPlugin ${Boost_LIBRARIES} d3d9.lib)

set_target_properties(CubemapExtractionPlugin
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Lib"
)