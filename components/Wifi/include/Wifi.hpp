#ifndef WIFI_HPP
#define WIFI_HPP

extern "C" {
#include "esp_err.h"
#include "esp_event_base.h"
}

#include <map>
#include <string>

namespace lea {

    class Wifi {
      public:
        /**
         * @brief enum to know the current wifi status
         */
        typedef enum
        {
            CONNECTED,
            DISCONNECTED,
            STARTED,
            STOPPED
        } wifi_status_e;

        /**
         * @brief Create a new wifi object
         */
        Wifi();

        /**
         * @brief Initialize wifi (in STA mode)
         *
         * @return ESP_OK or ESP_ERR if an error occured
         */
        esp_err_t init();

        /**
         * @brief connect to an AP of the list of ssid/password (actually call _connect to connect)
         *
         * @param knowWifiList map of x knowns pairs of ssid/password
         *
         * @return ESP_OK or ESP_ERR if an error occured
         */
        esp_err_t connect(const std::string &ssid, const std::string &passwd);

        /**
         * @brief reconnect to last AP
         *
         * @return ESP_OK or ESP_ERR if an error occured
         */
        esp_err_t reconnect();

        /**
         * @brief disconnect from AP
         *
         * @return ESP_OK or ESP_ERR if an error occured
         */
        esp_err_t disconnect();

        /**
         * @brief get current wifi status
         *
         * @return wifi status
         */
        lea::Wifi::wifi_status_e getStatus() const;

        /**
         * @brief Stop all wifi
         *
         * @return ESP_OK or ESP_ERR if an error occured
         */
        esp_err_t stop();

      private:
        /**
         * @brief connect to a wifi AP
         *
         * @param ssid ssid of the AP
         * @param passwd password of the AP
         *
         * @return ESP_OK or ESP_ERR if an error occured
         */
        void _connect(const std::string &ssid, const std::string &passwd);

        /**
         * @brief event handler when connecting to an AP, no need to document the parameters since it's standard
         *
         */
        static void _event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

        /**
         * @brief connection status
         */
        wifi_status_e _status;

        /**
         * @brief ssid of the current AP
         */
        std::string _currentSsid;

        /**
         * @brief password of the current AP
         */
        std::string _currentPasswd;
    };
}  // namespace lea

#endif  // WIFI_HPP
