set(SOURCES
    main.cpp
)
	
set(HEADERS
)

find_package(Boost
  1.54                  # Minimum version
  REQUIRED              # Fail with error if Boost is not found
  COMPONENTS thread date_time system chrono filesystem program_options regex # Boost libraries by their canonical name
)                     # e.g. "date_time" for "libboost_date_time"

add_executable(TrafficMonitor
	${SOURCES}
	${HEADERS}
)
target_include_directories(TrafficMonitor
	PRIVATE
	${Boost_INCLUDE_DIRS}
)
target_link_libraries(TrafficMonitor
	${Boost_LIBRARIES}
    #AlloReceiver
	AlloShared
)

set_target_properties(TrafficMonitor
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Bin/${CMAKE_BUILD_TYPE}"
)