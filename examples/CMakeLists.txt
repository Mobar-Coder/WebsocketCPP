cmake_minimum_required(VERSION 3.10)

include_directories(${CMAKE_SOURCE_DIR})

project(WebsocketCppServerExample)
add_executable(${PROJECT_NAME} ServerTest.cpp ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBS} WebsocketCpp)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unknown-pragmas)

project(WebSocketCppClientExample)
add_executable(${PROJECT_NAME} ClientTest.cpp ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBS} WebsocketCpp)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unknown-pragmas)
