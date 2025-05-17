#ifndef RECORDER_HPP
#define RECORDER_HPP

extern "C" {
#include "audio_common.h"
#include "audio_pipeline.h"
#include "esp_err.h"
}

#include <string>

namespace lea {
    class Recorder {
      public:
        Recorder();
        esp_err_t init();
        esp_err_t deinit();
        esp_err_t recordInFile(const std::string &fp);
        esp_err_t stopRecording();
        void turnOn();
        void turnOff();
        void setState(bool state);
        bool getState() const;

      private:
        bool _state;
        audio_pipeline_handle_t pipeline;
        audio_event_iface_handle_t evt;

        audio_element_handle_t fatfs_stream_writer, i2s_stream_reader, audio_encoder;
    };
}  // namespace lea

#endif  // RECORDER_HPP
