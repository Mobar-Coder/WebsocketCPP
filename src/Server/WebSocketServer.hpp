/**
 * @file WebSocketServer.hpp
 * @author paul
 * @date 10.03.19
 * @brief Declares the WebSocketServer class
 */

#ifndef SOPRANETWORK_WEBSOCKETSERVER_HPP
#define SOPRANETWORK_WEBSOCKETSERVER_HPP

#include <cstdint>
#include <thread>
#include <atomic>
#include <map>
#include <vector>

#include <libwebsockets.h>

#include "Listener.hpp"
#include "Connection.hpp"

namespace websocketcpp {
    /**
     * The WebSocketServer is the main class for handling of connnections, it manages the events
     * and generates an event for every new connection.
     */
    class WebSocketServer {
        friend class Connection;
    public:
        /**
         * CTor: Create a new server.
         * @param port the port to use, often this is port 80 (HTTP)
         * @param protocolName the name of the protocol to use (needs to be the same as in the client code)
         */
        WebSocketServer(uint16_t port, std::string protocolName);

        /**
         * This listener is triggered every time a new connection exists. The handlers are
         * called in the server thread, not in the main thread!
         */
        const util::Listener<std::shared_ptr<Connection>> connectionListener;

        /**
         * This listener is triggered every time a connection is closed. The handlers are
         * called in the server thread, not in the main thread!
         */
        const util::Listener<std::shared_ptr<Connection>> closeListener;

        /**
         * Send a message to all connected clients. The data is not send immediatly but when the server
         * thread is not busy (should be usually less than 50ms)
         * @param text the message to send
         */
        void broadcast(const std::string& text);

        /**
         * DTor
         */
        ~WebSocketServer();
    private:
        void run();
        int handler(lws *websocket, lws_callback_reasons reasons, int *id, const std::string& text);

        std::thread workerThread;
        std::atomic_bool finished;

        std::string protocolName;
        std::unique_ptr<lws_context, decltype(&lws_context_destroy)> context;
        const std::vector<lws_protocols> protocols;

        AsyncCallListPtr callList;
        int connectionUidCount;
        std::map<int, std::shared_ptr<Connection>> connections;

        static int globalHandler(lws *websocket, lws_callback_reasons reasons, void *userData, void *data,
                                 std::size_t len);
        static std::map<lws_context*, WebSocketServer*> instances;
        static void sendImpl(std::string text, lws *wsi);
    };

}

#endif //SOPRANETWORK_WEBSOCKETSERVER_HPP
