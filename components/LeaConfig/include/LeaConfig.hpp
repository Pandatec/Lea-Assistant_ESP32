#ifndef LEACONFIG_HPP
#define LEACONFIG_HPP

extern "C" {
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/touch_pad.h"
#include "driver/uart.h"
#include "esp_err.h"
}

// LYRAT
#define GREEN_LED_PIN (GPIO_NUM_22)

#define TOUCH_BUTTON_PLAY (TOUCH_PAD_NUM8)
#define TOUCH_BUTTON_SET (TOUCH_PAD_NUM9)
#define TOUCH_BUTTON_VOL_MINUS (TOUCH_PAD_NUM4)
#define TOUCH_BUTTON_VOL_PLUS (TOUCH_PAD_NUM7)

static const char PATH_TO_CONFIG_FILE[] = "/sdcard/config.txt";

static const gpio_num_t BUTTON_1_PIN = GPIO_NUM_39;
static const gpio_num_t BUTTON_2_PIN = GPIO_NUM_36;

#endif  // LEACONFIG_HPP
