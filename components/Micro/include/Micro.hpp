#ifndef MICRO_HPP
#define MICRO_HPP

extern "C" {
#include "audio_common.h"
#include "audio_pipeline.h"
#include "esp_err.h"
#include "esp_vad.h"
}

#include "Recorder.hpp"
#include "Vad.hpp"

#include <string>

namespace lea {

    class Micro {
      public:
        Micro();
        esp_err_t init();
        esp_err_t deinit();
        esp_err_t recordInFile(const std::string &fp);
        void turnOn();
        void turnOff();
        void setState(bool state);
        bool getState() const;
        bool detectVoice();

      private:
        Vad _vad;
        Recorder _rec;
        bool _state;
    };

}  // namespace lea

#endif  // MICRO_HPP
