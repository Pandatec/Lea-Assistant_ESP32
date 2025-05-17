#ifndef VAD_HPP
#define VAD_HPP

extern "C" {
#include "audio_common.h"
#include "audio_pipeline.h"
#include "esp_err.h"
#include "esp_vad.h"
}

#include <string>

namespace lea {
    class Vad {
      public:
        Vad();
        esp_err_t init();
        esp_err_t deinit();
        bool detectVoice();
        esp_err_t recordInFile(const std::string &fp);
        void turnOn();
        void turnOff();
        void setState(bool state);
        bool getState() const;

      private:
        bool _state;
        bool _speechDetected;

        audio_pipeline_handle_t pipeline;
        audio_element_handle_t i2s_stream_reader, filter, raw_read;
        vad_handle_t vad_inst;
        int16_t *vad_buff;
    };
}  // namespace lea

#endif  // VAD_HPP
