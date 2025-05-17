#include "WebSockets.hpp"

extern "C" {
#include "esp_event.h"
#include "esp_log.h"
#include "esp_websocket_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
}

#include <cstring>
#include <nlohmann/json.hpp>
#include <type_traits>

static const char *TAG = "WebSocket";

#define WEB_SERVER "192.168.1.89"
#define WEB_PORT "8080"

lea::WebSocket::WebSocket()
{
}

void lea::WebSocket::websocket_event_handler(esp_event_base_t base, int32_t event_id, void *event_data)
{
    const esp_websocket_event_data_t &data = *reinterpret_cast<esp_websocket_event_data_t *>(event_data);

    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
            break;
        case WEBSOCKET_EVENT_DATA:
            //ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
            //ESP_LOGI(TAG, "Received opcode=%d", data.op_code);
            if (data.op_code == 0x01)  // connection et authentification
            {
                char buff[data.data_len + 1];
                std::memcpy(buff, data.data_ptr, data.data_len);
                buff[data.data_len] = 0;
                auto json = nlohmann::json::parse(std::string(buff));
                if (json["type"] == "error") {
                    ESP_LOGE(TAG, "error : %s", std::string(json["data"]).c_str());
                }
                if (json["type"] == "token")  // 1st connection
                {
                    nvs_handle_t handle;
                    ESP_ERROR_CHECK(nvs_open("nvs", NVS_READWRITE, &handle));
                    ESP_ERROR_CHECK(nvs_set_str(handle, "token", std::string(json["data"]).c_str()));
                    ESP_ERROR_CHECK(nvs_commit(handle));
                    nvs_close(handle);
                    ESP_LOGI(TAG, "first connection, you are connected");
                } else if (json["type"] == "tokenAccepted")  //login
                {
                    ESP_LOGI(TAG, "welcome back, you are connected");
                } else if (json["type"] == "enableLocation")  //enable connection
                {
                    ESP_LOGI(TAG, "enableLocation");
                } else if (json["type"] == "disableLocation")  //disable connection
                {
                    ESP_LOGI(TAG, "disableLocation");
                }
            } else if (data.op_code == 0x02)  //transmission d'audio
            {
                ESP_LOGI(TAG, "data len %u", data.data_len);
                //ESP_LOGW(TAG, "NOT_BEGIN");
            } else {
                //ESP_LOGW(TAG, "Received=%u %.*s", data.op_code, data.data_len, data.data_ptr);
            }

            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
            break;
    }
}

void lea::WebSocket::connect()
{
    nvs_handle_t handle;
    auto ret = nvs_open("nvs", NVS_READONLY, &handle);
    if (ret == ESP_OK) {
        size_t size = 257;
        char token[size];
        ret = nvs_get_str(handle, "token", token, &size);
        ESP_ERROR_CHECK(ret);
        if (std::string(token).empty()) {
            nlohmann::json json;
            json["type"] = "firstConnexion";
            json["data"] = "";
            auto data = json.dump(0);
            esp_websocket_client_send_text(_client, data.c_str(), data.length(), 1000);
        } else {
            nlohmann::json json;
            json["type"] = "login";
            json["data"] = std::string(token).c_str();
            auto data = json.dump(0);
            esp_websocket_client_send_text(_client, data.c_str(), data.length(), 1000);
        }
    } else {
        ESP_LOGE(TAG, "Error connect");
    }
    nvs_close(handle);
}

void lea::WebSocket::disconnect()
{
    // TODO
    esp_websocket_client_close(_client, 1000);
}

template <typename Fn> static esp_err_t esp_websocket_register_events_lb(
    esp_websocket_client_handle_t client, esp_websocket_event_id_t event, Fn &&fn)
{
    return esp_websocket_register_events(
        client, event,
        [](void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
            auto &fn = *reinterpret_cast<std::remove_reference_t<Fn> *>(handler_args);
            fn(base, event_id, event_data);
        },
        &fn);
}

esp_err_t lea::WebSocket::init()
{
    esp_websocket_client_config_t ws_cfg{};
    ws_cfg.uri = "ws://" WEB_SERVER ":" WEB_PORT "/dev";
    ws_cfg.task_stack = 8192 * 2;

    ESP_LOGI(TAG, "Connecting to %s...", ws_cfg.uri);

    _client = esp_websocket_client_init(&ws_cfg);
    if (!_client) {
        ESP_LOGE(TAG, "WebSocket client init failed");
        return ESP_FAIL;
    }

    _handler = [this](esp_event_base_t base, int32_t event_id, void *event_data) {
        websocket_event_handler(base, event_id, event_data);
    };
    esp_websocket_register_events_lb(_client, WEBSOCKET_EVENT_ANY, _handler);

    esp_websocket_client_start(_client);
    ESP_LOGI(TAG, "Client start correctly");

    connect();

    return ESP_OK;
}

lea::WebSocket::~WebSocket()
{
}
