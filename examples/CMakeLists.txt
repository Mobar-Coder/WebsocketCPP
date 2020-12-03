cmake_minimum_required(VERSION 3.10)

include_directories(${CMAKE_SOURCE_DIR})

project(ServerTest)
add_executable(${PROJECT_NAME} ServerTest.cpp ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBS} WebsocketCpp)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-error=unknown-pragmas)

project(ClientTest)
add_executable(${PROJECT_NAME} ClientTest.cpp ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${LIBS} WebsocketCpp)
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-error=unknown-pragmas)
