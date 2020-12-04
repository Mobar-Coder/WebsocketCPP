/**
 * @file ClientTest.cpp
 * @author paul
 * @date 13.03.19
 * @brief Small test/example program for the WebSocketClient
 */
#include <iostream>
#include <chrono>
#include <thread>

#include "../src/Client/WebSocketClient.hpp"


int main() {
    using namespace std::chrono_literals;

    websocketcpp::WebSocketClient client{"localhost", "/", 8080, "http-only"};

    auto handler = [](std::string s){
        std::cout << s << std::endl;
    };

    client.receiveListener.subscribe(handler);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        std::this_thread::sleep_for(1s);
        client.send("Test");
    }
#pragma clang diagnostic pop
}
