cmake_minimum_required (VERSION 2.6)
project (tunnler CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RAKNET_PREFIX "${PROJECT_SOURCE_DIR}/externals/RakNet")
include_directories(${RAKNET_PREFIX}/Source)
add_subdirectory(${RAKNET_PREFIX})

include_directories("${PROJECT_SOURCE_DIR}/include")

set(SRCS
    src/room.cpp
    src/room_member.cpp
    )

set(HEADERS
    include/tunnler/room.h
    include/tunnler/room_member.h
    include/tunnler/room_message_types.h
    include/tunnler/tunnler.h
    )

add_library(tunnler STATIC ${SRCS} ${HEADERS})
target_link_libraries(tunnler RakNetLibStatic)

add_subdirectory(src/samples)