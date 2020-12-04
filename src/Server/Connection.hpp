/**
 * @file Connection.hpp
 * @author paul
 * @date 10.03.19
 * @brief Declares the Connection class
 */

#ifndef SOPRANETWORK_CONNECTION_HPP
#define SOPRANETWORK_CONNECTION_HPP

#include <memory>
#include <mutex>

#include <libwebsockets.h>
#include <sstream>

#include "Listener.hpp"

namespace websocketcpp {
    using AsyncCallListPtr = std::shared_ptr<std::pair<std::list<std::function<void()>>, std::mutex>>;

    /**
     * A connection represents one client of the server. This class cannot be instantiated,
     * all instances get created by a WebSocketServer.
     */
    class Connection {
        friend class WebSocketServer;
        public:
            /**
             * CTor: Not default constructable
             */
            Connection() = delete;

            /**
             * CTor: not copyable
             */
            Connection(const Connection&) = delete;

            /**
             * Listener which is called every time a new message is received.
             */
            const util::Listener<std::string> receiveListener;

            /**
             * Send a string to the client. The data is not send immediatly but when the server thread
             * is not busy (should be usually less than 50ms)
             * @param text the text to send
             * @throws std::runtime_error if the client disconnected
             */
            void send(const std::string& text);

            /**
             * Checks if the connection is still valid and a client is connected.
             * @return true if the connection is valid
             */
            auto isValid() const -> bool;
        private:
            Connection(lws* socket, AsyncCallListPtr asyncCallList);

            void receiveAndDefragment(const std::string &message, bool isLastMessage);
            lws* socket;
            AsyncCallListPtr callList;
            std::stringstream receiveStream;
    };
}

#endif //SOPRANETWORK_CONNECTION_HPP
