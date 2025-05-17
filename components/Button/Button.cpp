#include "Button.hpp"

lea::Button::Button(gpio_num_t pin, gpio_pull_direction_e direction)
{
    _pin = pin;
    _direction = direction;

    gpio_config_t conf = {
        //
        .pin_bit_mask = (1ULL << _pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = (direction == UP) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = (direction == DOWN) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&conf);
}

lea::Button::~Button()
{
}

bool lea::Button::getState() const
{
    /* if (_direction == UP) { */
    /*     return !gpio_get_level(_pin); */
    /* } else { */
    /*     return gpio_get_level(_pin); */
    /* } */
    return !gpio_get_level(_pin);
}
