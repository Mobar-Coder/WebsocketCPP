/**
 * @file WebSocketClient.cpp
 * @author paul
 * @date 13.03.19
 * @brief Implements the WebSocketClient class
 */

#include "WebSocketClient.hpp"

#include <utility>

namespace network {
    std::map<lws_context*, WebSocketClient*> WebSocketClient::instances;

    WebSocketClient::WebSocketClient(const std::string &server, const std::string &path,
            uint16_t port, const std::string &protocolName) :
        finished{false},
        connected{false},
        server{server}, path{path}, port{port},
        protocolName{protocolName},
        context{nullptr, lws_context_destroy},
        protocols{
            {
                this->protocolName.c_str(),
                &WebSocketClient::globalHandler,
                0,
                4096,
                0,
                nullptr,
                0
            },
            {
                nullptr, nullptr, 0, 0, 0, nullptr, 0 // Quasi null terminator
            }
        } {

        lws_context_creation_info contextCreationInfo{};
        contextCreationInfo.port = CONTEXT_PORT_NO_LISTEN;
        contextCreationInfo.protocols = this->protocols.data();
        contextCreationInfo.gid = -1;
        contextCreationInfo.uid = -1;

        this->context = decltype(this->context){lws_create_context(&contextCreationInfo),
                                                lws_context_destroy};

        if (!this->context) {
            throw std::runtime_error("Could not initialize websocket");
        }
        instances.insert({this->context.get(), this});

        lws_client_connect_info clientConnectInfo{};
        clientConnectInfo.context = this->context.get();
        clientConnectInfo.port = this->port;
        clientConnectInfo.address = this->server.c_str();
        clientConnectInfo.path = this->path.c_str();
        clientConnectInfo.host = this->server.c_str();
        clientConnectInfo.origin = this->server.c_str();
        clientConnectInfo.ssl_connection = false;
        clientConnectInfo.protocol = this->protocolName.c_str();
        clientConnectInfo.local_protocol_name = this->protocolName.c_str();
        clientConnectInfo.pwsi = &this->wsi;

        if (!lws_client_connect_via_info(&clientConnectInfo)) {
            throw std::runtime_error("Could not connect!");
        }

        this->workerThread = std::thread{&WebSocketClient::run, this};
    }

    void WebSocketClient::send(std::string text) {
        if (this->finished) {
            throw std::runtime_error("Connection already closed!");
        }
        std::lock_guard<std::mutex> lockGuard{callList.second};
        callList.first.emplace_back([=](){WebSocketClient::sendImpl(text, this->wsi);});
    }

    WebSocketClient::~WebSocketClient() {
        this->finished = true;
        this->workerThread.join();
        WebSocketClient::instances.erase(this->context.get());
    }

    void WebSocketClient::run() {
        while (!finished) {
            lws_service(this->context.get(), 50);

            if (connected) {
                std::lock_guard<std::mutex> lock{this->callList.second};
                for (const auto &call : this->callList.first) {
                    call();
                }
                this->callList.first.clear();
            }
        }
    }

    int WebSocketClient::handler(lws_callback_reasons reasons, std::string text) {
        switch (reasons) {
            case LWS_CALLBACK_CLIENT_ESTABLISHED:
                this->connected = true;
                break;
            case LWS_CALLBACK_CLIENT_RECEIVE: {
                    const std::size_t remaining = lws_remaining_packet_payload(this->wsi);
                    const bool isFinalFragment = lws_is_final_fragment(this->wsi);

                    this->receiveStream << text;
                    if (!remaining && isFinalFragment) {
                        this->receiveListener(receiveStream.str());
                        receiveStream.str(std::string{});
                    }
                }
                break;
            case LWS_CALLBACK_CLOSED:
                this->finished = true;
                closeListener();
                break;
            default:
                break;
        }
        return 0;
    }

    int WebSocketClient::globalHandler(lws *websocket, lws_callback_reasons reasons, void*, void *data,
                                       std::size_t len) {
        std::string text{static_cast<char *>(data), len};
        auto *ctx = lws_get_context(websocket);
        auto instance = instances.find(ctx);
        if (instance != instances.end()) {
            return instance->second->handler(reasons, text);
        }
        return 0;
    }

    void WebSocketClient::sendImpl(std::string text, lws *wsi) {
        lws_write(wsi,
                  reinterpret_cast<unsigned char*>(text.data()), text.length(), LWS_WRITE_TEXT);
    }

}
