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

# make exes and libs profilable on windows
if(MSVC)
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG
		"${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /PROFILE"
	)
	set(CMAKE_STATIC_LINKER_FLAGS_DEBUG
		"${CMAKE_STATIC_LINKER_FLAGS_DEBUG} /PROFILE"
	)
	set(CMAKE_MODULE_LINKER_FLAGS_DEBUG
		"${CMAKE_MODULE_LINKER_FLAGS_DEBUG} /PROFILE"
	)
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG
		"${CMAKE_EXE_LINKER_FLAGS_DEBUG} /PROFILE"
	)
endif()

add_subdirectory(AlloShared)
add_subdirectory(AlloServer)
add_subdirectory(CubemapExtractionPlugin)
add_subdirectory(AlloPlayer)