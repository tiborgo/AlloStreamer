cmake_minimum_required (VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

project(AlloPresenter)

# Boost setup
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_STATIC_LIBS OFF)
add_definitions(-DBOOST_ALL_DYN_LINK)

# Use unicode in every project
add_definitions(-DUNICODE -D_UNICODE)

# In case the libraries have to be connected to Unity
set(UNITY_PROJECT_PLUGIN_DIR "${CMAKE_SOURCE_DIR}/AlloStreamer/Assets/Plugins")

# so that each project can include the other project's headers
include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(AlloShared)
add_subdirectory(AlloServer)
add_subdirectory(CubemapExtractionPlugin)