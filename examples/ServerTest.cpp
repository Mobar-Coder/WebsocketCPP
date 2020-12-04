/**
 * @file serverTest.cpp
 * @author paul
 * @date 11.03.19
 * @brief Small websockets server test/example
 */
#include <iostream>

#include "../src/Server/WebSocketServer.hpp"

void newConnectionHandler(const std::shared_ptr<websocketcpp::Connection> &connection) {
    std::cout << "New Connection!" << std::endl;
    connection->send("Pong");

    connection->receiveListener.subscribe([connection](const std::string &text) {
        std::cout << "Received: " << text << std::endl;
        connection->send("Echo");
    });
}

int main() {
    websocketcpp::WebSocketServer server{8080, "http-only"};
    server.connectionListener.subscribe(newConnectionHandler);
    std::cout << "Started on port 8080" << std::endl;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {}
#pragma clang diagnostic pop
}

