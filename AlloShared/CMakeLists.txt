set(SOURCES
	CubemapFace.cpp
)
	
set(HEADERS
	CubemapFace.h
)

find_package(Boost
  1.57                  # Minimum version
)

include_directories(${Boost_INCLUDE_DIRS})
add_library(AlloShared SHARED ${SOURCES} ${HEADERS})