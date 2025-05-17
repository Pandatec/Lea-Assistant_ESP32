extern "C" {
#include "audio_hal.h"
#include "board.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
}

#include "Utils.hpp"
#include "lea.hpp"

static const char *TAG = "INIT";

namespace lea {

    void readConfig(const std::string &fp, std::map<std::string, std::string> &config)
    {
        auto vec = SD::getFileContent(fp);

        if (vec.empty()) {
            ESP_LOGE(TAG, "Config file is empty");
            ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
        }

        for (auto &l : vec) {
            if (!l.size() /* empty line */ || l.at(0) == '#' /* comment */) {  // skip line
                continue;
            }

            std::vector<std::string> splittedLine;
            utils::split(l, "=", splittedLine);

            if (splittedLine.size() != 2) {
                ESP_LOGE(TAG, "Invalid line : %s", l.c_str());
                continue;
            }
            config[splittedLine.at(0)] = splittedLine.at(1);
        }
    }

    std::unique_ptr<data_t> init()
    {
        // init nvs
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

        nvs_handle_t handle;
        ESP_ERROR_CHECK(nvs_open("nvs", NVS_READWRITE, &handle));
        size_t size = 257;
        char token[size];
        ret = nvs_get_str(handle, "token", token, &size);
        if (ret != ESP_OK) {
            ESP_LOGI(TAG, "Writing default token in nvs");
            ESP_ERROR_CHECK(nvs_set_str(handle, "token", ""));
            ESP_ERROR_CHECK(nvs_commit(handle));
        }
        nvs_close(handle);

        // init data struct
        auto data = std::make_unique<data_t>();

        if (!data) {
            ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
        }

        // init audio codec, must be after i2c init
        auto board_handle = audio_board_init();
        audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);

        data->player = std::make_unique<AudioPlayer>();
        ESP_ERROR_CHECK(data->player->init());

        data->mic = std::make_unique<Micro>();
        ESP_ERROR_CHECK(data->mic->init());

        ESP_ERROR_CHECK(SD::init());

        readConfig("/sdcard/config.txt", data->config);
        utils::printStringMap("Config map:", data->config);

        data->wifi = std::make_unique<Wifi>();
        ESP_ERROR_CHECK(data->wifi->init());

        data->webSocket = std::make_unique<WebSocket>();
        /* ESP_ERROR_CHECK(data->webSocket->init()); */

        return data;
    }
}  // namespace lea
