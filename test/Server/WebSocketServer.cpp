#include <gtest/gtest.h>

#include <src/Server/WebSocketServer.hpp>

TEST(WebSocketServer, ConstructDeconstruct) {
    EXPECT_NO_THROW((websocketcpp::WebSocketServer{8080, "abc"}));
    EXPECT_NO_THROW((websocketcpp::WebSocketServer{8081, "http-only"}));
}
