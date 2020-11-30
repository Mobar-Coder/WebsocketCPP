/**
 * @file serverTest.cpp
 * @author paul
 * @date 11.03.19
 * @brief Small websockets server test/example
 */
#include <iostream>

#include "../src/Server/WebSocketServer.hpp"

// Or use the installed version:
//#include <SopraNetwork/WebSocketServer.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void newConnectionHandler(std::shared_ptr<websocket::network::Connection> connection) {
    std::cout << "New Connection!" << std::endl;
    connection->send("Pong");

    connection->receiveListener.subscribe([connection](std::string text) {
        std::cout << "Received: " << text << std::endl;
        connection->send("Echo");
    });
}

int main() {
    websocket::network::WebSocketServer server{8080, "http-only"};
    server.connectionListener.subscribe(newConnectionHandler);
    std::cout << "Started on port 8080" << std::endl;
    while (true) {

    }
}

#pragma clang diagnostic pop