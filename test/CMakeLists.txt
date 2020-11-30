project(WebsocketCppTest)

enable_testing()
include_directories(..)
find_package(GTest)
if (GTest_FOUND)
    include_directories(${GTEST_INCLUDE_DIR})

    set(TEST_SOURCES
            Server/WebSocketServer.cpp
            Client/WebSocketClient.cpp)

    if (USE_INSTALLED_LIB)
        add_definitions(-DUSE_INSTALLED_LIB)
        add_executable(${PROJECT_NAME} main.cpp ${TEST_SOURCES})
        target_link_libraries(${PROJECT_NAME} gmock gtest pthread ${CMAKE_PROJECT_NAME})
    else()
        include_directories(${CMAKE_SOURCE_DIR})
        add_executable(${PROJECT_NAME} main.cpp ${SOURCES} ${TEST_SOURCES})
        target_link_libraries(${PROJECT_NAME} ${LIBS} gmock gtest pthread)
    endif()

    add_test(
            NAME ${PROJECT_NAME}
            COMMAND ${PROJECT_NAME}
    )
endif()

