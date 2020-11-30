#include <gtest/gtest.h>

#ifdef USE_INSTALLED_LIB
#include <SopraNetwork/Server/WebSocketServer.hpp>
#else
#include <src/Server/WebSocketServer.hpp>
#endif

TEST(WebSocketServer, ConstructDeconstruct) {
    EXPECT_NO_THROW((websocket::network::WebSocketServer{8080, "abc"}));
    EXPECT_NO_THROW((websocket::network::WebSocketServer{8081, "http-only"}));
}
