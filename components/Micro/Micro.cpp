#include "Micro.hpp"

extern "C" {
#include "board.h"
#include "esp_log.h"
#include "filter_resample.h"
#include "freertos/FreeRTOS.h"
#include "i2s_stream.h"
#include "raw_stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

static const char *TAG = "MICRO";

lea::Micro::Micro() : _state(false)
{
}

esp_err_t lea::Micro::init()
{
    if (_vad.init() != ESP_OK) {
        ESP_LOGE(TAG, "VAD init failed");
        return ESP_FAIL;
    }

    _rec.init();

    return ESP_OK;
}

esp_err_t lea::Micro::deinit()
{
    return ESP_OK;
}

esp_err_t lea::Micro::recordInFile(const std::string &fp)
{
    _rec.recordInFile(fp);
    return ESP_OK;
}

void lea::Micro::turnOn()
{
    _state = true;
}

void lea::Micro::turnOff()
{
    _state = false;
}

void lea::Micro::setState(bool state)
{
    if (state) {
        turnOn();
        _state = true;
    } else {
        turnOff();
        _state = false;
    }
}

bool lea::Micro::getState() const
{
    return _state;
}

bool lea::Micro::detectVoice()
{
    return _vad.detectVoice();
}
