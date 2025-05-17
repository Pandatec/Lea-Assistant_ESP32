#ifndef TOUCHBUTTON_HPP
#define TOUCHBUTTON_HPP

#include "driver/gpio.h"
#include "driver/touch_pad.h"

namespace lea {
    class TouchButton {
    public:
        TouchButton(touch_pad_t pin);
        bool getState() const;

    private:
        touch_pad_t _pin;
    };
} // namespace lea

#endif /* TOUCHBUTTON_HPP */
