# Try to find Live555 libraries
# Once done this will define
#  Live555_FOUND
#  Live555_INCLUDE_DIRS
#  Live555_LIBRARIES

# Uncomment the following line to print which directory CMake is looking in.
#MESSAGE(STATUS "PART4_ROOT_DIR: " ${PART4_ROOT_DIR})

if (NOT Live555_FOUND)
	
	set(_Live555_FOUND ON)
	
	foreach (library BasicUsageEnvironment groupsock liveMedia UsageEnvironment)

		find_path(Live555_${library}_INCLUDE_DIR
			NAMES ${library}.hh
			PATHS ${Live555_ROOT}/${library}/include
		)
		
		if (Live555_${library}_INCLUDE_DIR)
			list(APPEND _Live555_INCLUDE_DIRS ${Live555_${library}_INCLUDE_DIR})
		else()
			set(_Live555_FOUND OFF)
		endif ()

		find_library(Live555_${library}_LIBRARY
			NAMES lib${library}
			PATHS ${Live555_ROOT}/${library}
		)
		if (Live555_${library}_LIBRARY)
			list(APPEND _Live555_LIBRARIES ${Live555_${library}_LIBRARY})
		else()
			set(_Live555_FOUND OFF)
		endif ()

	endforeach ()

	if (_Live555_FOUND)
		set(Live555_INCLUDE_DIRS ${_Live555_INCLUDE_DIRS} CACHE INTERNAL "")
		set(Live555_LIBRARIES ${_Live555_LIBRARIES} CACHE INTERNAL "")
		set(Live555_FOUND ${_Live555_FOUND} CACHE BOOL "" FORCE)
	endif()

	include(FindPackageHandleStandardArgs)
	# handle the QUIETLY and REQUIRED arguments and set LOGGING_FOUND to TRUE
	# if all listed variables are TRUE
	find_package_handle_standard_args(Live555 DEFAULT_MSG Live555_INCLUDE_DIRS Live555_LIBRARIES Live555_FOUND)

	# Tell cmake GUIs to ignore the "local" variables.
	mark_as_advanced(Live555_INCLUDE_DIRS Live555_LIBRARIES Live555_FOUND)
endif (NOT Live555_FOUND)

if (Live555_FOUND)
	message(STATUS "Found live555")
endif()