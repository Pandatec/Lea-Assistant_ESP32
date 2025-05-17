#include "TouchButton.hpp"

#include "esp_log.h"

#define TOUCH_THRESH_NO_USE (0)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

lea::TouchButton::TouchButton(touch_pad_t pin) : _pin(pin)
{
    touch_pad_init();
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    touch_pad_config(_pin, TOUCH_THRESH_NO_USE);

    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
}

bool lea::TouchButton::getState() const
{
    uint16_t touch_value;

    touch_pad_read_raw_data(_pin, &touch_value);
    /* ESP_LOGI("lea", "%d", touch_value); */
    return touch_value < 400;
}
