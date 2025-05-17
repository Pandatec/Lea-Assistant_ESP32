extern "C" {
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
}

#include "Button.hpp"
#include "LeaConfig.hpp"
#include "Led.hpp"
#include "TouchButton.hpp"
#include "lea.hpp"

static const char *TAG = "MAIN";

lea::Led ledMicro(GREEN_LED_PIN);
lea::TouchButton t1(TOUCH_PAD_NUM8);

lea::Button buttonMicro(GPIO_NUM_39, lea::Button::UP);
lea::Button buttonSpeaker(GPIO_NUM_36, lea::Button::UP);

extern "C" void app_main()
{
    auto data = lea::init();

    lea::run(*data);

    ESP_ERROR_CHECK(ESP_FAIL);  // Code should never arrive here
}
