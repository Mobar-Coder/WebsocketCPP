cmake_minimum_required(VERSION 3.10)
project(WebsocketCpp VERSION 0.0.1 DESCRIPTION "Websocket library for C++")

# Build options
set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wpedantic -march=native -mtune=native")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Building for debug")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    message("Building for release")
endif ()

# Building

set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Server/WebSocketServer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Server/Connection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Client/WebSocketClient.cpp)

set(LIBS websockets pthread)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBS})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

# Installation via make install
include(GNUInstallDirs)
set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/WebsocketCpp
        FILES_MATCHING PATTERN "*.hpp")

add_subdirectory(test)
add_subdirectory(examples)
