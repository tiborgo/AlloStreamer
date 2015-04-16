set(SOURCES
	CubemapFace.cpp
)
	
set(HEADERS
	CubemapFace.h
)

find_package(Boost
  1.57                  # Minimum version
)

add_library(AlloShared
	SHARED
	${SOURCES} ${HEADERS}
)
target_include_directories(AlloShared
	PRIVATE
	${Boost_INCLUDE_DIRS}
)