#include "Vad.hpp"

extern "C" {
#include "board.h"
#include "esp_log.h"
#include "filter_resample.h"
#include "freertos/FreeRTOS.h"
#include "i2s_stream.h"
#include "raw_stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

static const char *TAG = "MICRO";

#define VAD_SAMPLE_RATE_HZ 16000
#define VAD_FRAME_LENGTH_MS 30

#define VAD_BUFFER_LENGTH (VAD_FRAME_LENGTH_MS * VAD_SAMPLE_RATE_HZ / 1000)

lea::Vad::Vad() : _state(false), _speechDetected(false)
{
}

esp_err_t lea::Vad::init()
{
    // Create audio pipeline for recording
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    // Create i2s stream to read audio data from codec chip
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.i2s_config.sample_rate = 48000;
    i2s_cfg.type = AUDIO_STREAM_READER;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);

    // Create filter to resample audio data
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = 48000;
    rsp_cfg.src_ch = 2;
    rsp_cfg.dest_rate = VAD_SAMPLE_RATE_HZ;
    rsp_cfg.dest_ch = 1;
    filter = rsp_filter_init(&rsp_cfg);

    // Create raw to receive data
    raw_stream_cfg_t raw_cfg = {
        .type = AUDIO_STREAM_READER,
        .out_rb_size = 8 * 1024,
    };
    raw_read = raw_stream_init(&raw_cfg);

    // Register all elements to audio pipeline
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
    audio_pipeline_register(pipeline, filter, "filter");
    audio_pipeline_register(pipeline, raw_read, "raw");

    // Link elements together [codec_chip]-->i2s_stream-->filter-->raw-->[VAD]
    const char *link_tag[3] = {"i2s", "filter", "raw"};
    audio_pipeline_link(pipeline, &link_tag[0], 3);

    // Start audio_pipeline
    audio_pipeline_run(pipeline);

    // Initialize VAD handle
    vad_inst = vad_create(VAD_MODE_1, VAD_SAMPLE_RATE_HZ, VAD_FRAME_LENGTH_MS);

    vad_buff = (int16_t *)malloc(VAD_BUFFER_LENGTH * sizeof(short));
    if (vad_buff == NULL) {
        ESP_LOGE(TAG, "Memory allocation failed!");
        ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    }
    return ESP_OK;
}

bool lea::Vad::detectVoice()
{
    ESP_LOGI(TAG, "CHECKING VOICE DETECTION...");

    raw_stream_read(raw_read, (char *)vad_buff, VAD_BUFFER_LENGTH * sizeof(short));

    // Feed samples to the VAD process and get the result
    vad_state_t vad_state = vad_process(vad_inst, vad_buff);
    if (vad_state == VAD_SPEECH) {
        ESP_LOGI(TAG, "Speech detected");
        _speechDetected = true;
    } else {
        _speechDetected = false;
    }
    return _speechDetected;
}

esp_err_t lea::Vad::deinit()
{
    // Destroy VAD
    vad_destroy(vad_inst);

    // Stop audio_pipeline and release all resources
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    audio_pipeline_unregister(pipeline, i2s_stream_reader);
    audio_pipeline_unregister(pipeline, filter);
    audio_pipeline_unregister(pipeline, raw_read);

    /* Release all resources */
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(i2s_stream_reader);
    audio_element_deinit(filter);
    audio_element_deinit(raw_read);

    free(vad_buff);
    vad_buff = NULL;
    return ESP_OK;
}

void lea::Vad::turnOn()
{
    audio_pipeline_run(pipeline);
    _state = true;
}

void lea::Vad::turnOff()
{
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);
    _state = false;
}

void lea::Vad::setState(bool state)
{
    if (state) {
        turnOn();
        _state = true;
    } else {
        turnOff();
        _state = false;
    }
}

bool lea::Vad::getState() const
{
    return _state;
}
