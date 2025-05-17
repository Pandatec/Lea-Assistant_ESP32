#include "Recorder.hpp"

extern "C" {
#include "audio_common.h"
#include "audio_element.h"
#include "audio_event_iface.h"
#include "audio_pipeline.h"
#include "board.h"
#include "es8388.h"
#include "esp_log.h"
#include "esp_peripherals.h"
#include "fatfs_stream.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2s_stream.h"
#include "opus_encoder.h"
#include "periph_sdcard.h"
#include "sdkconfig.h"

#include <string.h>
}

static const char *TAG = "RECORDER";

#define RECORD_TIME_SECONDS (5)

#define SAMPLE_RATE 48000
#define CHANNEL 1
#define BIT_RATE 64000
#define COMPLEXITY 10

lea::Recorder::Recorder()
{
}

esp_err_t lea::Recorder::init()
{
    ESP_LOGI(TAG, "[3.0] Create audio pipeline for recording");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[3.1] Create fatfs stream to write data to sdcard");
    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.type = AUDIO_STREAM_WRITER;
    fatfs_stream_writer = fatfs_stream_init(&fatfs_cfg);

    ESP_LOGI(TAG, "[3.2] Create i2s stream to read audio data from codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_READER;
    i2s_cfg.i2s_config.sample_rate = SAMPLE_RATE;
    if (CHANNEL == 1) {
        i2s_cfg.i2s_config.channel_format = I2S_CHANNEL_FMT_ALL_RIGHT;
    } else {
        i2s_cfg.i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
    }
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(TAG, "[3.3] Create audio encoder to handle data");
    opus_encoder_cfg_t opus_cfg = DEFAULT_OPUS_ENCODER_CONFIG();
    opus_cfg.sample_rate = SAMPLE_RATE;
    opus_cfg.channel = CHANNEL;
    opus_cfg.bitrate = BIT_RATE;
    opus_cfg.complexity = COMPLEXITY;
    audio_encoder = encoder_opus_init(&opus_cfg);

    ESP_LOGI(TAG, "[3.4] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");

    audio_pipeline_register(pipeline, audio_encoder, "opus");
    audio_pipeline_register(pipeline, fatfs_stream_writer, "file");

    ESP_LOGI(TAG, "[3.5] Link it together [codec_chip]-->i2s_stream-->opus_encoder-->fatfs_stream-->[sdcard]");
    const char *link_tag[3] = {"i2s", "opus", "file"};
    audio_pipeline_link(pipeline, &link_tag[0], 3);

    ESP_LOGI(TAG, "[ 4 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[4.1] Listening event from pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    es_mic_gain_t gain = MIC_GAIN_12DB;
    es8388_set_mic_gain(gain);

    return ESP_OK;
}

esp_err_t lea::Recorder::deinit()
{
    audio_pipeline_unregister(pipeline, audio_encoder);
    audio_pipeline_unregister(pipeline, i2s_stream_reader);
    audio_pipeline_unregister(pipeline, fatfs_stream_writer);

    /* Terminal the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);

    /* Release all resources */
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(fatfs_stream_writer);
    audio_element_deinit(i2s_stream_reader);
    audio_element_deinit(audio_encoder);
    return ESP_OK;
}

esp_err_t lea::Recorder::recordInFile(const std::string &fp)
{
    ESP_LOGI(TAG, "[3.6] Set up  uri (file as fatfs_stream, opus as opus encoder)");
    audio_element_set_uri(fatfs_stream_writer, fp.c_str());

    audio_pipeline_run(pipeline);

    ESP_LOGI(TAG, "[ 6 ] Listen for all pipeline events, record for %d Seconds", RECORD_TIME_SECONDS);
    int second_recorded = 0;
    while (1) {
        audio_event_iface_msg_t msg;
        if (audio_event_iface_listen(evt, &msg, 1000 / portTICK_RATE_MS) != ESP_OK) {
            second_recorded++;
            ESP_LOGI(TAG, "[ * ] Recording ... %d", second_recorded);
            if (second_recorded >= RECORD_TIME_SECONDS) {
                break;
            }
            continue;
        }

        /* Stop when the last pipeline element (i2s_stream_reader in this case) receives stop event */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *)i2s_stream_reader &&
            msg.cmd == AEL_MSG_CMD_REPORT_STATUS &&
            (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            ESP_LOGW(TAG, "[ * ] Stop event received");
            break;
        }
    }
    return ESP_OK;
}

esp_err_t lea::Recorder::stopRecording()
{
    ESP_LOGI(TAG, "[ 7 ] Stop audio_pipeline");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);
    return ESP_OK;
}

void lea::Recorder::turnOn()
{
    _state = true;
}

void lea::Recorder::turnOff()
{
    _state = false;
}

void lea::Recorder::setState(bool state)
{
    if (state) {
        turnOn();
        _state = true;
    } else {
        turnOff();
        _state = false;
    }
}

bool lea::Recorder::getState() const
{
    return _state;
}

