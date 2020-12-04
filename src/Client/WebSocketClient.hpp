/**
 * @file WebSocketClient.hpp
 * @author paul
 * @date 13.03.19
 * @brief Declares the WebSocketClient class
 */

#ifndef WEBSOCKETCPP_WEBSOCKETCLIENT_HPP
#define WEBSOCKETCPP_WEBSOCKETCLIENT_HPP

#include <string>
#include <thread>
#include <sstream>
#include <atomic>
#include <map>
#include <mutex>
#include <vector>

#include <libwebsockets.h>

#include "../Listener.hpp"

namespace websocketcpp {
    using AsyncCallList = std::pair<std::list<std::function<void()>>, std::mutex>;

    /**
     * Implements a client which can connect to a server
     */
    class WebSocketClient {
    public:
        /**
         * CTor, construct a new client and connect.
         * @param server the hostname of the server (example.com)
         * @param path the path on the server (/xyz still needs a "/" if empty)
         * @param port the port, usually 80 or 443
         * @param protocolName the name of the protocol
         */
        WebSocketClient(std::string server, std::string path,
                uint16_t port, std::string protocolName);

        /**
         * Listener which gets called everytime a new package is received. This listener
         * gets called in the server thread!
         */
        const util::Listener<std::string> receiveListener;

        /**
         * Listener which gets called when the client disconnects. This listener
         * gets called in the server thread!
         */
        const util::Listener<> closeListener;

        /**
         * Send a message to the server. The data is not send immediatly but when the server
         * thread is not busy (should be usually less than 50ms)
         * @param text the message to send
         */
        void send(const std::string& text);

        /**
         * DTor
         */
        ~WebSocketClient();
    private:
        void run();
        int handler(lws_callback_reasons reasons, const std::string& text);

        std::thread workerThread;
        std::atomic_bool finished;
        std::atomic_bool connected;

        std::string server, path;
        uint16_t port;

        /*
         * Even tho these variables might seem unused they are necessary to exist for the complete lifetime of
         * the object (lws uses the data which it gets via a raw pointer).
         */
        std::string protocolName;
        std::unique_ptr<lws_context, decltype(&lws_context_destroy)> context;
        lws *wsi; // Yes i know this is a raw pointer, but wsi doesn't need destruction.
        const std::vector<lws_protocols> protocols;

        AsyncCallList callList;

        std::stringstream receiveStream;

        static int globalHandler(lws *websocket, lws_callback_reasons reasons, void*, void *data,
                                 std::size_t len);
        static void sendImpl(const std::string &text, lws *wsi);
        static std::map<lws_context*, WebSocketClient*> instances;
    };
}


#endif
