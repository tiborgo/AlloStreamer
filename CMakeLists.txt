cmake_minimum_required (VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

project(AlloUnity)

set(ENABLE_ALLOSHARED ON CACHE BOOL "")
set(ENABLE_ALLOSERVER ON CACHE BOOL "")
set(ENABLE_CUBEMAPEXTRACTIONPLUGIN ON CACHE BOOL "")
set(ENABLE_UNITYRENDERINGPLUGIN ON CACHE BOOL "")
set(ENABLE_FAKEUNITY ON CACHE BOOL "")
set(ENABLE_ALLORECEIVER ON CACHE BOOL "")
set(ENABLE_ALLOPLAYER ON CACHE BOOL "")
set(ENABLE_WINDOWEDPLAYER ON CACHE BOOL "")
set(ENABLE_OCULUSPLAYER ON CACHE BOOL "")
set(ENABLE_TRAFFICMONITOR ON CACHE BOOL "")
set(ENABLE_ALLOSERVER_BINOCULARS ON CACHE BOOL "")
set(ENABLE_RENDERINGPLUGIN_BINOCULARS ON CACHE BOOL "")
set(ENABLE_UNITYSCRIPTS_BINOCULARS ON CACHE BOOL "")

# Boost setup
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_STATIC_LIBS OFF)
add_definitions(-DBOOST_ALL_DYN_LINK)
add_definitions(-DROOT_DIR="${CMAKE_SOURCE_DIR}")
if(WIN32)
    add_definitions(-DBOOST_INTERPROCESS_BOOTSTAMP_IS_LASTBOOTUPTIME) # prevents error when startup time is not in Windows sys log
	add_definitions(-DBOOST_INTERPROCESS_WINDOWS)
endif()

# Use unicode in every project
add_definitions(-DUNICODE -D_UNICODE)

# In case the libraries have to be connected to Unity
set(UNITY_PROJECT_ASSETS_DIR "${CMAKE_SOURCE_DIR}/AlloStreamer/Assets/" CACHE STRING "")
set(UNITY_PROJECT_PLUGIN_DIR "${UNITY_PROJECT_ASSETS_DIR}/Plugins" CACHE STRING "")

# so that each project can include the other project's headers
include_directories(${CMAKE_SOURCE_DIR})

# Enable C++11
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
	CMAKE_COMPILER_IS_GNUCXX)
   # We are using C++11 features and
   # we are linking static libs into shared libs
   # (AlloShared linked into CubemapExtractionPlugin),
   # thus the PIC.
   # The decision was made against making AlloShared a shared lib
   # since it needs to be loadable by the CubemapExtractionPlugin and, in turn, by Unity3D.
   # However, Unity3D doesn't give us the possibility to set the search path
   # for shared libs and makes it very cumbersome to load external shared libs.
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC")
endif()

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

if(ENABLE_ALLOSHARED)
	add_subdirectory(AlloShared)
endif()
if(ENABLE_ALLOSERVER)
	add_subdirectory(AlloServer)
endif()
if(ENABLE_CUBEMAPEXTRACTIONPLUGIN)
	add_subdirectory(CubemapExtractionPlugin)
endif()
if(ENABLE_UNITYRENDERINGPLUGIN)
	add_subdirectory(UnityRenderingPlugin)
endif()
if(ENABLE_FAKEUNITY)
	add_subdirectory(FakeUnity)
endif()
if(ENABLE_ALLORECEIVER)
    add_subdirectory(AlloReceiver)
endif()
if(ENABLE_ALLOPLAYER)
	add_subdirectory(AlloPlayer)
endif()
if(ENABLE_WINDOWEDPLAYER)
	add_subdirectory(WindowedPlayer)
endif()
if(ENABLE_OCULUSPLAYER)
	add_subdirectory(OculusPlayer)
endif()
if(ENABLE_TRAFFICMONITOR)
	add_subdirectory(TrafficMonitor)
endif()
if(ENABLE_ALLOSERVER_BINOCULARS)
	add_subdirectory(AlloServer_Binoculars)
endif()
if(ENABLE_RENDERINGPLUGIN_BINOCULARS)
	add_subdirectory(RenderingPlugin_Binoculars)
endif()
if(ENABLE_UNITYSCRIPTS_BINOCULARS)
	add_subdirectory(UnityScripts_Binoculars)
endif()
