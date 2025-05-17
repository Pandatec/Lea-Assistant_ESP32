#ifndef LED_HPP
#define LED_HPP

#include "driver/gpio.h"

namespace lea {
    class Led {
    public:
        Led(gpio_num_t pin);
        ~Led();
        void turnOn();
        void turnOff();
        void toggle();
        bool getState() const;
        void setState(bool newState);

    private:
        bool _state;
        gpio_num_t _pin;
    };
} // namespace lea

#endif // LED_HPP
