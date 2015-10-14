# Try to find OSCPack library
# Once done this will define
#  OSCPack_FOUND
#  OSCPack_INCLUDE_DIRS
#  OSCPack_LIBRARIES

find_path(OSCPack_INCLUDE_DIR
	NAMES
	osc/OscTypes.h
	PATHS
	${OSCPack_ROOT}
)

foreach (mode DEBUG RELEASE)
	find_library(OSCPack_LIBRARY_${mode}
		NAMES
		oscpack
		PATHS
		${OSCPack_ROOT}/Lib/${mode}
	)
	if (OSCPack_LIBRARY_${mode})
		if (${mode} STREQUAL RELEASE) 
			list(APPEND OSCPack_LIBRARIES optimized ${OSCPack_LIBRARY_${mode}})
		elseif (${mode} STREQUAL DEBUG) 
			list(APPEND OSCPack_LIBRARIES debug ${OSCPack_LIBRARY_${mode}})
		endif()
	endif ()
endforeach ()

if ((NOT ${OSCPack_INCLUDE_DIR} STREQUAL "")  AND (NOT ${OSCPack_LIBRARY_DEBUG} STREQUAL "") AND (NOT ${OSCPack_LIBRARY_RELEASE} STREQUAL ""))
	set(OSCPack_INCLUDE_DIRS ${OSCPack_INCLUDE_DIR} CACHE INTERNAL "")
	set(OSCPack_FOUND ON CACHE BOOL "" FORCE)
else()
	set(OSCPack_INCLUDE_DIRS "" CACHE INTERNAL "")
	set(OSCPack_LIBRARIES "" CACHE INTERNAL "")
	set(OSCPack_FOUND OFF CACHE BOOL "" FORCE)
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LOGGING_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(OSCPack DEFAULT_MSG OSCPack_INCLUDE_DIRS OSCPack_LIBRARIES OSCPack_FOUND)

# Tell cmake GUIs to ignore the "local" variables.
mark_as_advanced(OSCPack_INCLUDE_DIRS OSCPack_LIBRARIES OSCPack_FOUND)

if (OSCPack_FOUND)
	message(STATUS "Found OSCPack")
endif()
