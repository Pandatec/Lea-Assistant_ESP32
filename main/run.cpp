extern "C" {
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
}

#include "Button.hpp"
#include "LeaConfig.hpp"
#include "Led.hpp"
#include "lea.hpp"

static const char *TAG = "RUN";

lea::Button b1(BUTTON_2_PIN);
lea::Led l1(GPIO_NUM_22);

void lea::run(data_t &data)
{
    /* if (data.wifi->connect(data.config.at("WIFI_SSID"), data.config.at("WIFI_PASSWORD")) == ESP_OK) { */
    /*     data.webSocket->init(); */
    /* } */

    while (1) {
        if (data.mic->detectVoice()) {
            ESP_LOGI(TAG, "Voice detected");
            l1.turnOn();
            data.mic->recordInFile("/sdcard/tmp.opus");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            data.player->music_play("/sdcard/tmp.opus");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        } else {
            l1.turnOff();
        }
        /* ESP_LOGI(TAG, "..."); */
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
