set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")

add_executable(server server.cpp)
#target_link_libraries(server ${PLATFORM_LIBRARIES} Threads::Threads)
target_link_libraries(server tunnler)

add_executable(lobby lobby.cpp)
#target_link_libraries(lobby ${PLATFORM_LIBRARIES} Threads::Threads)
target_link_libraries(lobby tunnler)
