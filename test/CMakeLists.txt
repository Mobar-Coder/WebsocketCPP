project(WebsocketCppTest)

enable_testing()
include_directories(..)
find_package(GTest)
if (GTest_FOUND)
    include_directories(${GTEST_INCLUDE_DIR})

    set(TEST_SOURCES
            Server/WebSocketServer.cpp
            Client/WebSocketClient.cpp)

    include_directories(${CMAKE_SOURCE_DIR})
    add_executable(${PROJECT_NAME} main.cpp ${SOURCES} ${TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME} ${LIBS} gmock gtest pthread)

    add_test(
            NAME ${PROJECT_NAME}
            COMMAND ${PROJECT_NAME}
    )
endif()

