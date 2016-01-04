# From: http://stackoverflow.com/questions/18005880/how-to-writing-a-cmake-module-for-jsoncpp .
# - Try to find Jsoncpp
# Once done, this will define
#
#  Jsoncpp_FOUND - system has Jsoncpp
#  Jsoncpp_INCLUDE_DIRS - the Jsoncpp include directories
#  Jsoncpp_LIBRARIES - link these to use Jsoncpp

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Jsoncpp_PKGCONF jsoncpp)

# Include dir
find_path(Jsoncpp_INCLUDE_DIR
  NAMES json/features.h
  PATH_SUFFIXES jsoncpp include
  PATHS ${Jsoncpp_PKGCONF_INCLUDE_DIRS} ${Jsoncpp_ROOT}
)

# Finally the library itself
find_library(Jsoncpp_LIBRARY
  NAMES jsoncpp
  PATH_SUFFIXES lib
  PATHS ${Jsoncpp_PKGCONF_LIBRARY_DIRS} ${Jsoncpp_ROOT}
)

set(Jsoncpp_PROCESS_INCLUDES Jsoncpp_INCLUDE_DIR)
set(Jsoncpp_PROCESS_LIBS Jsoncpp_LIBRARY)
libfind_process(Jsoncpp)
