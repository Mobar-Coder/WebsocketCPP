/**
 * @file WebSocketServer.cpp
 * @author paul
 * @date 10.03.19
 * @brief Implements the WebSocketServer class
 */

#include <iostream>
#include "WebSocketServer.hpp"

static constexpr auto BUF_SIZE = 4096;

namespace websocketcpp {
    std::map<lws_context *, WebSocketServer *> WebSocketServer::instances;

    WebSocketServer::WebSocketServer(uint16_t port, std::string protocolName) :
            finished{false},
            protocolName{std::move(protocolName)},
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
            callList{std::make_shared<std::pair<std::list<std::function<void()>>, std::mutex>>()},
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
            for (std::size_t c = 0; c < text.size(); ++c) {
                buf[c + LWS_PRE] = text.at(c);
            }
            lws_write(wsi, buf.data() + LWS_PRE, text.length(), LWS_WRITE_TEXT); // NOLINT
        }
    }

    void WebSocketServer::broadcast(const std::string &text) {
        for (const auto &connection : this->connections) {
            connection.second->send(text);
        }
    }

    int WebSocketServer::handler(lws *websocket, lws_callback_reasons reasons, int *id, const std::string &text) {
        switch (reasons) {
            case LWS_CALLBACK_ESTABLISHED: {
                *id = ++connectionUidCount;
                // Yes i create a raw pointer and pass it to the shared_ptr ctor instead of using
                // make_shared, this is necessary because the Connector CTor is private.
                auto connection = std::shared_ptr<Connection>{new Connection{websocket, this->callList}};
                connections.emplace(std::make_pair(*id, connection));
                this->connectionListener(connection);
                break;
            }
            case LWS_CALLBACK_CLOSED: {
                auto it = connections.find(*id);
                if (it != connections.end()) {
                    it->second->socket = nullptr;
                    this->closeListener(it->second);
                    connections.erase(*id);
                }
                break;
            }
            case LWS_CALLBACK_RECEIVE: {
                auto it = connections.find(*id);
                if (it != connections.end()) {
                    const std::size_t remaining = lws_remaining_packet_payload(websocket);
                    const bool isFinalFragment = lws_is_final_fragment(websocket) != 0;

                    it->second->receiveAndDefragment(text, (remaining == 0) && isFinalFragment);
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
        auto *id = static_cast<int *>(userData);
        std::string text{static_cast<char *>(data), len};
        auto *ctx = lws_get_context(websocket);
        auto instance = instances.find(ctx);
        if (instance != instances.end()) {
            return instance->second->handler(websocket, reasons, id, text);
        }
        return 0;
    }
}
