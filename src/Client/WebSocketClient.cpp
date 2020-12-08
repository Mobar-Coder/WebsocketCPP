/**
 * @file WebSocketClient.cpp
 * @author paul
 * @date 13.03.19
 * @brief Implements the WebSocketClient class
 */

#include "WebSocketClient.hpp"

#include <utility>

static constexpr auto BUF_SIZE = 4096;

namespace websocketcpp {
    std::map<lws_context *, WebSocketClient *> WebSocketClient::instances;

    WebSocketClient::WebSocketClient(std::string server, std::string path,
                                     uint16_t port, std::string protocolName) :
            finished{false},
            connected{false},
            server{std::move(server)}, path{std::move(path)}, port{port},
            protocolName{std::move(protocolName)},
            context{nullptr, lws_context_destroy},
            wsi{nullptr},
            protocols{
                    {
                            this->protocolName.c_str(),
                                     &WebSocketClient::globalHandler,
                                              0,
                                                 BUF_SIZE,
                                                    0,
                                                       nullptr,
                                                                BUF_SIZE
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
        instances.emplace(this->context.get(), this);

        lws_client_connect_info clientConnectInfo{};
        clientConnectInfo.context = this->context.get();
        clientConnectInfo.port = this->port;
        clientConnectInfo.address = this->server.c_str();
        clientConnectInfo.path = this->path.c_str();
        clientConnectInfo.host = this->server.c_str();
        clientConnectInfo.origin = this->server.c_str();
        clientConnectInfo.ssl_connection = static_cast<int>(false);
        clientConnectInfo.protocol = this->protocolName.c_str();
        clientConnectInfo.local_protocol_name = this->protocolName.c_str();
        clientConnectInfo.pwsi = &this->wsi;

        if (nullptr == lws_client_connect_via_info(&clientConnectInfo)) {
            throw std::runtime_error("Could not connect!");
        }

        this->workerThread = std::thread{&WebSocketClient::run, this};
    }

    void WebSocketClient::send(const std::string &text) {
        if (this->finished) {
            throw std::runtime_error("Connection already closed!");
        }
        std::lock_guard<std::mutex> lockGuard{callList.second};
        callList.first.emplace_back([=]() { WebSocketClient::sendImpl(text, this->wsi); });
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

    int WebSocketClient::handler(lws_callback_reasons reasons, const std::string &text) {
        switch (reasons) {
            case LWS_CALLBACK_CLIENT_ESTABLISHED:
                this->connected = true;
                break;
            case LWS_CALLBACK_CLIENT_RECEIVE: {
                const std::size_t remaining = lws_remaining_packet_payload(this->wsi);
                const bool isFinalFragment = lws_is_final_fragment(this->wsi) != 0;

                this->receiveStream << text;
                if (remaining == 0 and isFinalFragment) {
                    this->receiveListener(receiveStream.str());
                    receiveStream.str(std::string{});
                }
            }
                break;
            case LWS_CALLBACK_CLIENT_CLOSED:
                this->finished = true;
                closeListener();
                break;
            default:
                break;
        }
        return 0;
    }

    int WebSocketClient::globalHandler(lws *websocket, lws_callback_reasons reasons, void *, void *data, // NOLINT
                                       std::size_t len) {
        std::string text{static_cast<char *>(data), len};
        auto *ctx = lws_get_context(websocket);
        auto instance = instances.find(ctx);
        if (instance != instances.end()) {
            return instance->second->handler(reasons, text);
        }
        return 0;
    }

    void WebSocketClient::sendImpl(const std::string &text, lws *wsi) {
        if (wsi != nullptr) {
            std::vector<unsigned char> buf;
            buf.resize(text.length() + LWS_PRE);
            for (std::size_t c = 0; c < text.size(); ++c) {
                buf[c + LWS_PRE] = text.at(c);
            }
            lws_write(wsi, buf.data() + LWS_PRE, text.length(), LWS_WRITE_TEXT); // NOLINT
        }
    }

}
