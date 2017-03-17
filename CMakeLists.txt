cmake_minimum_required (VERSION 2.6)
project (tunnler  C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package (PCAP REQUIRED)

include_directories("${PROJECT_SOURCE_DIR}/include" $PCAP_INCLUDE_DIR)
add_executable(tunnler src/main.cpp src/pcapHandle.cpp src/wiredManager.cpp)
set_target_properties(tunnler PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (tunnler  ${PCAP_LIBRARY})