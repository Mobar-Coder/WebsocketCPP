/**
 * @file WebSocketServer.cpp
 * @author paul
 * @date 10.03.19
 * @brief Implements the WebSocketServer class
 */

#include <iostream>
#include "WebSocketServer.hpp"

static constexpr auto BUF_SIZE = 8000000;

namespace network {
    std::map<lws_context*, WebSocketServer*> WebSocketServer::instances;

    WebSocketServer::WebSocketServer(uint16_t port, const std::string &protocolName) :
        finished{false},
        protocolName{protocolName},
        context{nullptr, lws_context_destroy},
        protocols{
            {
                this->protocolName.c_str(),
                &WebSocketServer::globalHandler,
                sizeof(int),
                BUF_SIZE,
                0,
                nullptr,
                BUF_SIZE 
            },
            {
                nullptr, nullptr, 0, 0, 0, nullptr, 0 // Quasi null terminator
            }
        },
        callList{std::make_shared<std::pair<std::list<std::function<void()>>,std::mutex>>()},
        connectionUidCount{0} {

        lws_context_creation_info contextCreationInfo{};
        contextCreationInfo.port = port;
        contextCreationInfo.protocols = this->protocols.data();
        contextCreationInfo.gid = -1;
        contextCreationInfo.uid = -1;

        this->context = decltype(this->context){lws_create_context(&contextCreationInfo),
                                                     lws_context_destroy};

        if (!this->context) {
            throw std::runtime_error("Could not initialize websocket");
        }
        instances.insert({this->context.get(), this});

        this->workerThread = std::thread{&WebSocketServer::run, this};
    }

    void WebSocketServer::run() {
        while (!finished) {
            lws_service(this->context.get(), 50);

            std::lock_guard<std::mutex> lock{this->callList->second};
            for (const auto &call : this->callList->first) {
                call();
            }
            this->callList->first.clear();
        }
    }

    void WebSocketServer::sendImpl(std::string text, lws *wsi) {
        if (wsi != nullptr) {
            std::vector<unsigned char> buf;
            buf.resize(text.length() + LWS_PRE);
            for (std::size_t c=0; c<text.size(); ++c) {
                buf[c + LWS_PRE] = text.at(c);
            }
            lws_write(wsi, buf.data() + LWS_PRE, text.length(), LWS_WRITE_TEXT);
        }
    }

    void WebSocketServer::broadcast(std::string text) {
        for (const auto &connection : this->connections) {
            connection.second->send(text);
        }
    }

    int WebSocketServer::handler(lws *websocket, lws_callback_reasons reasons, int *userData, std::string text) {
        switch (reasons) {
            case LWS_CALLBACK_ESTABLISHED: {
                *userData = ++connectionUidCount;
                // Yes i create a raw pointer and pass it to the shared_ptr ctor instead of using
                // make_shared, this is necessary because the Connector CTor is private.
                auto connection = std::shared_ptr<Connection>{new Connection{websocket, this->callList}};
                connections.emplace(std::make_pair(*userData, connection));
                this->connectionListener(connection);
                break;
            }
            case LWS_CALLBACK_CLOSED: {
                auto it = connections.find(*userData);
                if (it != connections.end()) {
                    it->second->socket = nullptr;
                    this->closeListener(it->second);
                    connections.erase(*userData);
                }
                break;
            }
            case LWS_CALLBACK_RECEIVE: {
                auto it = connections.find(*userData);
                if (it != connections.end()) {
                    const std::size_t remaining = lws_remaining_packet_payload(websocket);
                    const bool isFinalFragment = lws_is_final_fragment(websocket);

                    it->second->receiveAndDefragment(text, !remaining && isFinalFragment);
                }
                break;
            }
            default:
                break;
        }

        return 0;
    }

    WebSocketServer::~WebSocketServer() {
        this->finished = true;
        this->workerThread.join();
        WebSocketServer::instances.erase(this->context.get());
    }

    int WebSocketServer::globalHandler(lws *websocket, lws_callback_reasons reasons, void *userData, void *data,
                                       size_t len) {
        auto *id = static_cast<int*>(userData);
        std::string text{static_cast<char *>(data), len};
        auto *ctx = lws_get_context(websocket);
        auto instance = instances.find(ctx);
        if (instance != instances.end()) {
            return instance->second->handler(websocket, reasons, id, text);
        }
        return 0;
    }
}
