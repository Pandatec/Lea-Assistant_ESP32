#include "Wifi.hpp"

extern "C" {
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sys.h"
}

#include <string.h>

static const char *TAG = "Wifi";

/* FreeRTOS event group to signal when we are connected */
static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0  // IPC for wifi callback
#define WIFI_FAIL_BIT BIT1

// Retry stuff
#define MAX_RETRY_NUM 5
static int _retry_num = 0;

lea::Wifi::Wifi() : _status(STOPPED)
{
}

esp_err_t lea::Wifi::init()
{
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    _status = STARTED;

    return ESP_OK;
}

// callback function for wifi events
void lea::Wifi::_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (_retry_num < MAX_RETRY_NUM) {  // MAX NUM OF RETRY
            esp_wifi_connect();
            _retry_num++;
            ESP_LOGI(TAG, "retry %d to connect to the AP", _retry_num);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        _retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    } else {
        ESP_LOGE(TAG, "Unknown event in event handler");
    }
}

esp_err_t lea::Wifi::connect(const std::string &ssid, const std::string &passwd)
{
    // set up event handler
    s_wifi_event_group = xEventGroupCreate();
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &_event_handler, NULL, &instance_got_ip));

    ESP_LOGI(TAG, "Trying to connect to AP [%s] with password [%s] ...", ssid.c_str(), passwd.c_str());
    _connect(ssid, passwd);

    while (_status != CONNECTED) {  // wait for connection cb and if timeout go to next AP
        if (_retry_num == MAX_RETRY_NUM) {
            _retry_num = 0;
            break;
        }
    }

    if (_status == CONNECTED) {
        _currentSsid = ssid;
        _currentPasswd = passwd;
    } else {
        ESP_LOGE(TAG, "Could not connect to wifi");
        esp_wifi_stop();
    }

    // deinit event handler and
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);

    return _status == CONNECTED ? ESP_OK : ESP_FAIL;
}

void lea::Wifi::_connect(const std::string &ssid, const std::string &passwd)
{
    // connect to wifi
    wifi_config_t wifi_config = {};

    memcpy(wifi_config.sta.ssid, ssid.c_str(), strlen(ssid.c_str()));
    memcpy(wifi_config.sta.password, passwd.c_str(), strlen(passwd.c_str()));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits =
        xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s", ssid.c_str());
        _status = CONNECTED;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGW(TAG, "Failed to connect to SSID:%s, password:%s", ssid.c_str(), passwd.c_str());
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

esp_err_t lea::Wifi::reconnect()
{
    if (!_currentSsid.empty() && !_currentPasswd.empty()) {
        connect(_currentSsid, _currentPasswd);
        return _status == CONNECTED ? ESP_OK : ESP_FAIL;
    }

    ESP_LOGE(TAG, "No previous AP connection");
    return ESP_FAIL;
}

esp_err_t lea::Wifi::disconnect()
{
    esp_err_t ret;

    if (_status != CONNECTED) {
        ESP_LOGE(TAG, "Can't disconnect if not connected");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Disconnecting from wifi");

    ret = esp_wifi_disconnect();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Disconnection failed");
        return ret;
    }

    ret = esp_wifi_stop();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Stopping failed");
        return ret;
    }

    ESP_LOGI(TAG, "Disconnection OK");
    _status = DISCONNECTED;
    return ret;
}

esp_err_t lea::Wifi::stop()
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Stopping wifi ...");

    ret = esp_wifi_stop();
    if (ret == ESP_OK) {
        ESP_LOGE(TAG, "Stopping wifi failed");
        _status = STOPPED;
    }

    ESP_LOGI(TAG, "Stopping wifi OK");
    return ret;
}

lea::Wifi::wifi_status_e lea::Wifi::getStatus() const
{
    return _status;
}
