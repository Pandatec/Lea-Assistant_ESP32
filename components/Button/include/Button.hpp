#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "driver/gpio.h"

namespace lea {
    class Button {
    public:
        typedef enum { UP, DOWN, NONE } gpio_pull_direction_e;

        Button(gpio_num_t pin, gpio_pull_direction_e direction = NONE);
        ~Button();
        bool getState() const;

    private:
        gpio_pull_direction_e _direction;
        gpio_num_t _pin;
    };
} // namespace lea

#endif // BUTTON_HPP
