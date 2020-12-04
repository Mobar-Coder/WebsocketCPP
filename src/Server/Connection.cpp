/**
 * @file Connection.cpp
 * @author paul
 * @date 10.03.19
 * @brief Implements the Connection class
 */

#include <utility>
#include "Connection.hpp"
#include "WebSocketServer.hpp"

namespace websocketcpp {
    Connection::Connection(lws *socket, AsyncCallListPtr asyncCallList)
            : socket{socket}, callList{std::move(asyncCallList)}{}

    void Connection::send(const std::string& text) {
        if (!this->socket) {
            throw std::runtime_error("Client already disconnected");
        }
        std::lock_guard<std::mutex> lockGuard{callList->second};
        callList->first.emplace_back([=] { return WebSocketServer::sendImpl(text, socket); });
    }

    auto Connection::isValid() const -> bool {
        return !this->socket;
    }

    void Connection::receiveAndDefragment(const std::string &message, bool isLastMessage) {
        this->receiveStream << message;
        if (isLastMessage) {
            this->receiveListener(this->receiveStream.str());
            this->receiveStream.str(std::string{});
        }
    }
}
