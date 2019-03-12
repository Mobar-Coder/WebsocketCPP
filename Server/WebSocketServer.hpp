/**
 * @file WebSocketServer.hpp
 * @author paul
 * @date 10.03.19
 * @brief WebSocketServer @TODO
 */

#ifndef SOPRANETWORK_WEBSOCKETSERVER_HPP
#define SOPRANETWORK_WEBSOCKETSERVER_HPP

#include <cstdint>
#include <thread>
#include <mutex>
#include <atomic>

#include <libwebsockets.h>

#include "Connection.hpp"

namespace network {
    class WebSocketServer {
    public:
        using AsyncCallList = std::pair<std::shared_ptr<std::list<std::function<void()>>>,
                std::shared_ptr<std::mutex>>;

        explicit WebSocketServer(uint16_t port);

        const util::Listener<Connection> connectionListener;

        ~WebSocketServer();
    private:
        void sendImpl(std::string text, const std::unique_ptr<lws> &wsi);
        void run();

        std::thread workerThread;
        std::atomic_bool finished;
        AsyncCallList callList;

        std::unique_ptr<lws_context, decltype(&lws_context_destroy)> context;
        const std::vector<lws_protocols> protocols;

        static int handler(lws *websocket, lws_callback_reasons reasons, void * userData, void *data,
                std::size_t len);
    };
}

#endif //SOPRANETWORK_WEBSOCKETSERVER_HPP