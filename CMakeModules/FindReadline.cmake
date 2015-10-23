# Try to find readline library
# Once done this will define
#  Readline_FOUND
#  Readline_INCLUDE_DIRS
#  Readline_LIBRARIES

find_path(Readline_INCLUDE_DIR
	NAMES
	readline/readline.h
	HINTS
	${Readline_ROOT}
	PATH_SUFFIXES
	include
)

find_library(Readline_readline_LIBRARY
	NAMES
	readline
	HINTS
	${Readline_ROOT}/lib
)
find_library(Readline_history_LIBRARY
	NAMES
	history
	HINTS
	${Readline_ROOT}/lib
)



if ((NOT ${Readline_INCLUDE_DIR} STREQUAL "") AND (NOT ${Readline_readline_LIBRARY} STREQUAL "") AND (NOT ${Readline_history_LIBRARY} STREQUAL ""))
	set(Readline_INCLUDE_DIRS ${Readline_INCLUDE_DIR} CACHE INTERNAL "")
	set(Readline_FOUND ON CACHE BOOL "" FORCE)
	list(APPEND Readline_LIBRARIES ${Readline_readline_LIBRARY} ${Readline_history_LIBRARY})
else()
	set(Readline_INCLUDE_DIRS "" CACHE INTERNAL "")
	set(Readline_LIBRARIES "" CACHE INTERNAL "")
	set(Readline_FOUND OFF CACHE BOOL "" FORCE)
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LOGGING_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Readline DEFAULT_MSG Readline_INCLUDE_DIRS Readline_LIBRARIES Readline_FOUND)

# Tell cmake GUIs to ignore the "local" variables.
mark_as_advanced(Readline_INCLUDE_DIRS OSCPack_LIBRARIES Readline_LIBRARIES)

if (Readline_FOUND)
	message(STATUS "Found Readline")
endif()
