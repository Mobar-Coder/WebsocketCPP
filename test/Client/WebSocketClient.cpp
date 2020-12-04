#include <gtest/gtest.h>

#include <src/Client/WebSocketClient.hpp>

TEST(WebSocketClient, ConstructDestruct) {
    EXPECT_NO_THROW((websocketcpp::WebSocketClient{"localhost", "/", 8080, "test"}));
}
