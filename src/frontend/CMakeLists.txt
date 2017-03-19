set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")

find_package(Qt5 REQUIRED COMPONENTS Widgets ${QT_PREFIX_HINT})
set(FRONTEND_QT_LIBS Qt5::Widgets)

# Prefer the -pthread flag on Linux.
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


set(SRCS
            main.cpp
            room_list.cpp
            room_view_window.cpp
            room_list_window.cpp
            )

set(HEADERS
            main.h
            room_list.h
            room_list_p.h
            room_view_window.h
            room_list_window.h
            )

#create_directory_groups(${SRCS} ${HEADERS})

add_executable(frontend ${SRCS} ${HEADERS})
target_link_libraries(frontend ${FRONTEND_QT_LIBS})
target_link_libraries(frontend ${PLATFORM_LIBRARIES} Threads::Threads)


