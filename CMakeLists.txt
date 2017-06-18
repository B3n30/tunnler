cmake_minimum_required (VERSION 2.6)
project (tunnler CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENET_PREFIX "${PROJECT_SOURCE_DIR}/externals/enet")
include_directories(${ENET_PREFIX}/include)
add_subdirectory(${ENET_PREFIX})

include_directories("${PROJECT_SOURCE_DIR}/include")

set(SRCS
    src/bytestream.cpp
    src/room.cpp
    src/room_member.cpp
    )

set(HEADERS
    include/tunnler/bytestream.h
    include/tunnler/room.h
    include/tunnler/room_member.h
    include/tunnler/room_message_types.h
    include/tunnler/tunnler.h
    )

add_library(tunnler STATIC ${SRCS} ${HEADERS})
target_link_libraries(tunnler enet)
