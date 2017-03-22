cmake_minimum_required (VERSION 2.6)
project (tunnler CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RAKNET_PREFIX "${PROJECT_SOURCE_DIR}/externals/RakNet")
include_directories(${RAKNET_PREFIX}/Source)
add_subdirectory(${RAKNET_PREFIX})

set(JSON_PREFIX "${PROJECT_SOURCE_DIR}/externals/json")
include_directories(${JSON_PREFIX}/src)

#set(RESTCLIENT_PREFIX "${PROJECT_SOURCE_DIR}/externals/RestClient")
#include_directories(${RESTCLIENT_PREFIX}/include)
#add_subdirectory(${RESTCLIENT_PREFIX})

#set(CPPRESTSDK_PREFIX "${PROJECT_SOURCE_DIR}/externals/cpprestsdk")
#include_directories(${CPPRESTSDK_PREFIX}/Release/include)
#add_subdirectory(${CPPRESTSDK_PREFIX}/Release)

set(USE_SYSTEM_CURL ON CACHE BOOL "")
set(BUILD_CPR_TESTS OFF CACHE BOOL "")
set(CPR_PREFIX "${PROJECT_SOURCE_DIR}/externals/cpr")
add_subdirectory(${CPR_PREFIX})
include_directories(${CPR_INCLUDE_DIRS})

include_directories("${PROJECT_SOURCE_DIR}/include")

set(SRCS
    src/room.cpp
    src/room_member.cpp
    src/lobby.cpp
    )

set(HEADERS
    include/tunnler/room.h
    include/tunnler/room_member.h
    include/tunnler/room_message_types.h
    include/tunnler/lobby.h
    include/tunnler/tunnler.h
    )

add_library(tunnler STATIC ${SRCS} ${HEADERS})
target_link_libraries(tunnler RakNetLibStatic ${CPR_LIBRARIES})

add_subdirectory(src/frontend)
add_subdirectory(src/samples)
