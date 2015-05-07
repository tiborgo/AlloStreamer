# Try to find Live555 libraries
# Once done this will define
#  Live555_FOUND
#  Live555_INCLUDE_DIRS
#  Live555_LIBRARIES

# Uncomment the following line to print which directory CMake is looking in.
#MESSAGE(STATUS "PART4_ROOT_DIR: " ${PART4_ROOT_DIR})

if (NOT X264_FOUND)

	find_path(X264_INCLUDE_DIR
		NAMES x264.h
		PATHS ${X264_ROOT}
	)

	find_library(X264_LIBRARY
		NAMES x264
		PATHS
		${X264_ROOT}
		/usr/local/lib
	)

	if (X264_INCLUDE_DIR AND X264_LIBRARY)
		set(X264_INCLUDE_DIRS ${X264_INCLUDE_DIR} CACHE INTERNAL "")
		set(X264_LIBRARIES ${X264_LIBRARY} CACHE INTERNAL "")
		set(X264_FOUND ON CACHE BOOL "" FORCE)
	endif()

	include(FindPackageHandleStandardArgs)
	# handle the QUIETLY and REQUIRED arguments and set LOGGING_FOUND to TRUE
	# if all listed variables are TRUE
	find_package_handle_standard_args(X264 DEFAULT_MSG X264_INCLUDE_DIRS X264_LIBRARIES X264_FOUND)

	# Tell cmake GUIs to ignore the "local" variables.
	mark_as_advanced(X264_INCLUDE_DIRS X264_LIBRARIES X264_FOUND)
endif ()

if (X264_FOUND)
	message(STATUS "Found x264")
endif()
