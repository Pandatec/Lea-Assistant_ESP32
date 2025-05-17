#include "Led.hpp"

lea::Led::Led(gpio_num_t pin)
{
    _pin = pin;

    gpio_config_t conf = {
        //
        .pin_bit_mask = (1ULL << _pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&conf);
    this->turnOff();
}

lea::Led::~Led()
{
    //
}

void lea::Led::setState(bool newState)
{
    gpio_set_level(_pin, newState);
}

void lea::Led::turnOn()
{
    _state = true;
    this->setState(_state);
}

void lea::Led::turnOff()
{
    _state = false;
    this->setState(_state);
}

void lea::Led::toggle()
{
    _state = !_state;
    this->setState(_state);
}

bool lea::Led::getState() const
{
    return _state;
}
