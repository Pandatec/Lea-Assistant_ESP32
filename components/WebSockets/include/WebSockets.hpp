#ifndef WEBSOCKET_HPP_
#define WEBSOCKET_HPP_

extern "C" {
#include "esp_err.h"
#include "esp_websocket_client.h"
}

#include <functional>

namespace lea {
    class WebSocket {
      public:
        WebSocket();
        ~WebSocket();

        esp_err_t init();
        void connect();
        void disconnect();

      private:
        esp_websocket_client_handle_t _client;

        std::function<void(esp_event_base_t base, int32_t event_id, void *event_data)> _handler;
        void websocket_event_handler(esp_event_base_t base, int32_t event_id, void *event_data);
    };
}  // namespace lea
#endif /* !WEBSOCKET_HPP_ */
