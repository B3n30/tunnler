cmake_minimum_required (VERSION 2.6)
project (tunnler  C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package (PCAP REQUIRED)

set(RAKNET_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/externals/rakNet")
include_directories(${RAKNET_PREFIX}/source)
add_subdirectory(${RAKNET_PREFIX})

include_directories("${PROJECT_SOURCE_DIR}/include" $PCAP_INCLUDE_DIR)

add_executable(tunnler src/main.cpp src/pcapHandle.cpp src/room.cpp)
set_target_properties(tunnler PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (tunnler RakNetLibStatic ${PCAP_LIBRARY})

add_executable(tunnler_server src/server.cpp)
target_link_libraries (tunnler_server RakNetLibStatic)
