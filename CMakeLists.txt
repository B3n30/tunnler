cmake_minimum_required (VERSION 2.6)
project (tunnler  C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package (PCAP REQUIRED)
find_package (RakNet REQUIRED)

include_directories("${PROJECT_SOURCE_DIR}/include" ${PCAP_INCLUDE_DIR} ${RAKNET_INCLUDE_DIR}/raknet)

add_executable(tunnler src/main.cpp src/pcapHandle.cpp src/wiredManager.cpp) # src/room.cpp)
set_target_properties(tunnler PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (tunnler ${PCAP_LIBRARY} ${RAKNET_LIBRARIES})

add_executable(tunnler_server src/server.cpp)
target_link_libraries (tunnler_server ${RAKNET_LIBRARIES})
