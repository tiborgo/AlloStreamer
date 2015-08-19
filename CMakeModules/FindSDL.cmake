# Try to find SDL library
# Once done this will define
#  SDL_FOUND
#  SDL_INCLUDE_DIR
#  SDL_LIBRARY

find_path(SDL_INCLUDE_DIR
	NAMES
	SDL.h
	PATHS
	${SDL_ROOT}/include
	PATH_SUFFIXES
	SDL2
)


if (CMAKE_SIZEOF_VOID_P STREQUAL "8")
	set(SDL_ARCHITECTURE "x64")
else()
	set(SDL_ARCHITECTURE "x32")
endif()

find_library(SDL_LIBRARY
	NAMES
	SDL2
	PATHS
	${SDL_ROOT}/lib/${SDL_ARCHITECTURE}
)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LOGGING_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(SDL DEFAULT_MSG SDL_INCLUDE_DIR SDL_LIBRARY)
set(SDL_FOUND ON)

# Tell cmake GUIs to ignore the "local" variables.
mark_as_advanced(SDL_INCLUDE_DIR SDL_LIBRARY SDL_FOUND)

message(STATUS "Found SDL")
