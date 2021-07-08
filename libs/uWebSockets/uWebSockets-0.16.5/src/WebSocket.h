/*
 * Authored by Alex Hultman, 2018-2019.
 * Intellectual property of third-party.

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 *     http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UWS_WEBSOCKET_H
#define UWS_WEBSOCKET_H

#include "WebSocketData.h"
#include "WebSocketProtocol.h"
#include "AsyncSocket.h"
#include "WebSocketContextData.h"

#include <string_view>

namespace uWS {

template <bool SSL, bool isServer>
struct WebSocket : AsyncSocket<SSL> {
    template <bool> friend struct TemplatedApp;
private:
    typedef AsyncSocket<SSL> Super;

    void *init(bool perMessageDeflate, bool slidingCompression, std::string &&backpressure) {
        new (us_socket_ext(SSL, (us_socket_t *) this)) WebSocketData(perMessageDeflate, slidingCompression, std::move(backpressure));
        return this;
    }
public:

    /* Returns pointer to the per socket user data */
    void *getUserData() {
        WebSocketData *webSocketData = (WebSocketData *) us_socket_ext(SSL, (us_socket_t *) this);
        /* We just have it overallocated by sizeof type */
        return (webSocketData + 1);
    }

    /* See AsyncSocket */
    using Super::getBufferedAmount;
    using Super::getRemoteAddress;

    /* Simple, immediate close of the socket. Emits close event */
    using Super::close;

    /* Send or buffer a WebSocket frame, compressed or not. Returns false on increased user space backpressure. */
    bool send(std::string_view message, uWS::OpCode opCode = uWS::OpCode::BINARY, bool compress = false) {
        /* Every send resets the timeout */
        WebSocketContextData<SSL> *webSocketContextData = (WebSocketContextData<SSL> *) us_socket_context_ext(SSL,
            (us_socket_context_t *) us_socket_context(SSL, (us_socket_t *) this)
        );
        AsyncSocket<SSL>::timeout(webSocketContextData->idleTimeout);

        /* Transform the message to compressed domain if requested */
        if (compress) {
            WebSocketData *webSocketData = (WebSocketData *) Super::getAsyncSocketData();

            /* Check and correct the compress hint */
            if (opCode < 3 && webSocketData->compressionStatus == WebSocketData::ENABLED) {
                LoopData *loopData = Super::getLoopData();
                /* Compress using either shared or dedicated deflationStream */
                if (webSocketData->deflationStream) {
                    message = webSocketData->deflationStream->deflate(loopData->zlibContext, message, false);
                } else {
                    message = loopData->deflationStream->deflate(loopData->zlibContext, message, true);
                }
            } else {
                compress = false;
            }
        }

        /* Get size, alloate size, write if needed */
        size_t messageFrameSize = protocol::messageFrameSize(message.length());
        auto[sendBuffer, requiresWrite] = Super::getSendBuffer(messageFrameSize);
        protocol::formatMessage<isServer>(sendBuffer, message.data(), message.length(), opCode, message.length(), compress);
        if (requiresWrite) {
            auto[written, failed] = Super::write(sendBuffer, messageFrameSize);

            /* For now, we are slow here (fix!) */
            free(sendBuffer);

            /* Return true for success */
            return !failed;
        }

        /* Return success */
        return true;
    }

    /* Send websocket close frame, emit close event, send FIN if successful */
    void end(int code, std::string_view message = {}) {
        /* Check if we already called this one */
        WebSocketData *webSocketData = (WebSocketData *) us_socket_ext(SSL, (us_socket_t *) this);
        if (webSocketData->isShuttingDown) {
            return;
        }

        /* We postpone any FIN sending to either drainage or uncorking */
        webSocketData->isShuttingDown = true;

        /* Format and send the close frame */
        static const int MAX_CLOSE_PAYLOAD = 123;
        int length = std::min<size_t>(MAX_CLOSE_PAYLOAD, message.length());
        char closePayload[MAX_CLOSE_PAYLOAD + 2];
        int closePayloadLength = protocol::formatClosePayload(closePayload, code, message.data(), length);
        bool ok = send(std::string_view(closePayload, closePayloadLength), OpCode::CLOSE);

        /* FIN if we are ok and not corked */
        WebSocket<SSL, true> *webSocket = (WebSocket<SSL, true> *) this;
        if (!webSocket->isCorked()) {
            if (ok) {
                /* If we are not corked, and we just sent off everything, we need to FIN right here.
                 * In all other cases, we need to fin either if uncork was successful, or when drainage is complete. */
                webSocket->shutdown();
            }
        }

        /* Emit close event */
        WebSocketContextData<SSL> *webSocketContextData = (WebSocketContextData<SSL> *) us_socket_context_ext(SSL,
            (us_socket_context_t *) us_socket_context(SSL, (us_socket_t *) this)
        );
        if (webSocketContextData->closeHandler) {
            webSocketContextData->closeHandler(this, code, message);
        }

        /* Make sure to unsubscribe from any pub/sub node at exit */
        webSocketContextData->topicTree.unsubscribeAll(webSocketData->subscriber);
        delete webSocketData->subscriber;
        webSocketData->subscriber = nullptr;
    }

    /* Subscribe to a topic according to MQTT rules and syntax */
    void subscribe(std::string_view topic) {
        WebSocketContextData<SSL> *webSocketContextData = (WebSocketContextData<SSL> *) us_socket_context_ext(SSL,
            (us_socket_context_t *) us_socket_context(SSL, (us_socket_t *) this)
        );

        /* Make us a subscriber if we aren't yet */
        WebSocketData *webSocketData = (WebSocketData *) us_socket_ext(SSL, (us_socket_t *) this);
        if (!webSocketData->subscriber) {
            webSocketData->subscriber = new Subscriber(this);
        }

        webSocketContextData->topicTree.subscribe(topic, webSocketData->subscriber);
    }

    /* Unsubscribe from a topic, returns true if we were subscribed */
    bool unsubscribe(std::string_view topic) {
        WebSocketContextData<SSL> *webSocketContextData = (WebSocketContextData<SSL> *) us_socket_context_ext(SSL,
            (us_socket_context_t *) us_socket_context(SSL, (us_socket_t *) this)
        );

        WebSocketData *webSocketData = (WebSocketData *) us_socket_ext(SSL, (us_socket_t *) this);

        return webSocketContextData->topicTree.unsubscribe(topic, webSocketData->subscriber);
    }

    /* Publish a message to a topic according to MQTT rules and syntax */
    void publish(std::string_view topic, std::string_view message, OpCode opCode = OpCode::TEXT, bool compress = false) {
        WebSocketContextData<SSL> *webSocketContextData = (WebSocketContextData<SSL> *) us_socket_context_ext(SSL,
            (us_socket_context_t *) us_socket_context(SSL, (us_socket_t *) this)
        );
        /* Is the same as publishing per websocket context */
        webSocketContextData->publish(topic, message, opCode, compress);
    }
};

}

#endif // UWS_WEBSOCKET_H
