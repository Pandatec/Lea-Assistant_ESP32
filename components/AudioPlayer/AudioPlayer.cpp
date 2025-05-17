extern "C" {
#include "audio_error.h"
#include "audio_mem.h"
#include "board.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "fatfs_stream.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"
#include "opus_decoder.h"
#include "wav_decoder.h"
}

#include "AudioPlayer.hpp"

#include <string>

static const char *TAG = "MP3_PLAYER";

bool lea::AudioPlayer::isInit = false;

static void esp_audio_callback_func(esp_audio_state_t *audio, void *ctx)
{
    (void)ctx;

    ESP_LOGI(TAG, "ESP_AUDIO_CALLBACK_FUNC, st:%d,err:%d,src:%x", audio->status, audio->err_msg, audio->media_src);
}

lea::AudioPlayer::AudioPlayer()
{
}

esp_err_t lea::AudioPlayer::init()
{
    if (isInit) {
        return ESP_OK;
    }
    isInit = true;
    esp_audio_cfg_t cfg = DEFAULT_ESP_AUDIO_CONFIG();
    audio_board_handle_t board_handle = audio_board_get_handle();

    _player = (esp_player_t *)audio_calloc(1, sizeof(esp_player_t));
    if (!_player) {
        ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    }

    cfg.in_stream_buf_size = 4096;
    cfg.out_stream_buf_size = 4096;
    cfg.vol_handle = board_handle->audio_hal;
    cfg.vol_set = (audio_volume_set)audio_hal_set_volume;
    cfg.vol_get = (audio_volume_get)audio_hal_get_volume;
    cfg.resample_rate = 48000;
    cfg.prefer_type = ESP_AUDIO_PREFER_MEM;
    cfg.cb_func = esp_audio_callback_func;
    cfg.cb_ctx = NULL;
    _player->handle = esp_audio_create(&cfg);
    if (!_player->handle) {
        ESP_ERROR_CHECK(ESP_ERR_NO_MEM);
    }

    // Create readers and add to esp_audio
    fatfs_stream_cfg_t fs_reader = FATFS_STREAM_CFG_DEFAULT();
    fs_reader.type = AUDIO_STREAM_READER;

    esp_audio_input_stream_add(_player->handle, fatfs_stream_init(&fs_reader));

    // Add decoders and encoders to esp_audio

    // mp3
    /* mp3_decoder_cfg_t mp3_dec_cfg = DEFAULT_MP3_DECODER_CONFIG(); */
    /* mp3_dec_cfg.task_core = 1; */
    /* esp_audio_codec_lib_add(_player->handle, AUDIO_CODEC_TYPE_DECODER, mp3_decoder_init(&mp3_dec_cfg)); */

    // wav
    /* wav_decoder_cfg_t wav_dec_cfg = DEFAULT_WAV_DECODER_CONFIG(); */
    /* wav_dec_cfg.task_core = 1; */
    /* esp_audio_codec_lib_add(_player->handle, AUDIO_CODEC_TYPE_DECODER, wav_decoder_init(&wav_dec_cfg)); */

    // opus
    opus_decoder_cfg_t opus_decoder_cfg = DEFAULT_OPUS_DECODER_CONFIG();
    opus_decoder_cfg.task_core = 1;
    esp_audio_codec_lib_add(_player->handle, AUDIO_CODEC_TYPE_DECODER, decoder_opus_init(&opus_decoder_cfg));

    // Create writers and add to esp_audio
    i2s_stream_cfg_t i2s_writer = I2S_STREAM_CFG_DEFAULT();
    i2s_writer.type = AUDIO_STREAM_WRITER;
    i2s_writer.i2s_config.sample_rate = 48000;
    i2s_writer.i2s_config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    i2s_writer.i2s_config.channel_format = I2S_CHANNEL_FMT_ALL_RIGHT;

    esp_audio_output_stream_add(_player->handle, i2s_stream_init(&i2s_writer));

    AUDIO_MEM_SHOW(TAG);
    ESP_LOGI(TAG, "esp_audio instance is:%p", _player->handle);

    vol_set(100);

    return ESP_OK;
}

lea::AudioPlayer::~AudioPlayer()
{
    audio_free(_player);
}

audio_err_t lea::AudioPlayer::music_play(const std::string &fp)
{
    const std::string full_path = "file:/" + fp;
    ESP_LOGI(TAG, "Playing file :%s", full_path.c_str());

    audio_err_t ret = music_play(full_path.c_str(), 0, MEDIA_SRC_TYPE_MUSIC_SD);
    switch (ret) {
        case ESP_ERR_AUDIO_NO_ERROR:
            ESP_LOGI(TAG, "PLAY : OK");
            break;
        case ESP_ERR_AUDIO_TIMEOUT:
            ESP_LOGI(TAG, "PLAY : TIMEOUT");
            break;
        case ESP_ERR_AUDIO_NOT_SUPPORT:
            ESP_LOGI(TAG, "PLAY : NOT_SUPPORT");
            break;
        case ESP_ERR_AUDIO_INVALID_PARAMETER:
            ESP_LOGI(TAG, "PLAY : INVALID_PARAMETER");
            break;
        case ESP_ERR_AUDIO_INVALID_URI:
            ESP_LOGI(TAG, "PLAY : INVALID_URI");
            break;
        case ESP_ERR_AUDIO_STOP_BY_USER:
            ESP_LOGI(TAG, "PLAY : STOP_BY_USER");
            break;
        default:
            ESP_LOGI(TAG, "DEFAULT MATCH");
            break;
    }
    return ret;
}

audio_err_t lea::AudioPlayer::music_play(const char *url, int pos, media_source_type_t type)
{
    AUDIO_MEM_CHECK(TAG, _player, return ESP_ERR_AUDIO_MEMORY_LACK);
    esp_audio_state_t st;
    audio_err_t ret;

    esp_audio_state_get(_player->handle, &st);
    if (st.status == AUDIO_STATUS_RUNNING) {
        ret = esp_audio_stop(_player->handle, TERMINATION_TYPE_NOW);
        if (ret != ESP_ERR_AUDIO_NO_ERROR) {
            return ret;
        }
    }
    esp_audio_media_type_set(_player->handle, type);
    ret = esp_audio_play(_player->handle, AUDIO_CODEC_TYPE_DECODER, url, pos);
    return ret;
}

audio_err_t lea::AudioPlayer::music_stop()
{
    AUDIO_MEM_CHECK(TAG, _player, return ESP_ERR_AUDIO_MEMORY_LACK);
    audio_err_t ret = esp_audio_media_type_set(_player->handle, MEDIA_SRC_TYPE_NULL);
    ret = esp_audio_stop(_player->handle, TERMINATION_TYPE_NOW);
    return ret;
}

audio_err_t lea::AudioPlayer::music_pause()
{
    AUDIO_MEM_CHECK(TAG, _player, return ESP_ERR_AUDIO_MEMORY_LACK);
    audio_err_t ret = ESP_ERR_AUDIO_NO_ERROR;
    esp_audio_state_t st;

    esp_audio_state_get(_player->handle, &st);
    if (st.status == AUDIO_STATUS_RUNNING) {
        ret = esp_audio_pause(_player->handle);
        ret = esp_audio_info_get(_player->handle, &_player->backup_info);
        _player->backup_flag = true;
        ESP_LOGD(TAG, "%s, i:%p, c:%p, f:%p, o:%p,status:%d", __func__, _player->backup_info.in_el,
            _player->backup_info.codec_el, _player->backup_info.filter_el, _player->backup_info.out_el,
            _player->backup_info.st.status);
        return ret;
    } else {
        return ret;
    }
}

audio_err_t lea::AudioPlayer::music_resume()
{
    AUDIO_MEM_CHECK(TAG, _player, return ESP_ERR_AUDIO_MEMORY_LACK);
    audio_err_t ret = ESP_ERR_AUDIO_NO_ERROR;

    if (_player->backup_flag) {
        esp_audio_info_set(_player->handle, &_player->backup_info);
        _player->backup_flag = false;
        ret = esp_audio_resume(_player->handle);
    } else {
        ret = esp_audio_resume(_player->handle);
    }
    return ret;
}

audio_err_t lea::AudioPlayer::state_get(esp_audio_state_t *state)
{
    AUDIO_MEM_CHECK(TAG, _player, return ESP_ERR_AUDIO_MEMORY_LACK);
    return esp_audio_state_get(_player->handle, state);
}

void lea::AudioPlayer::vol_get(int *vol)
{
    esp_err_t ret = esp_audio_vol_get(_player->handle, vol);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error getting volume");
    }
}

void lea::AudioPlayer::vol_set(int vol)
{
    if (_player->handle == NULL) {
        ESP_LOGE(TAG, "Handle is NULL");
    }

    esp_err_t ret = esp_audio_vol_set(_player->handle, vol);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error setting volume");
    }
}

audio_err_t lea::AudioPlayer::get_state(esp_audio_state_t *state)
{
    AUDIO_MEM_CHECK(TAG, _player, return ESP_ERR_AUDIO_MEMORY_LACK);
    return esp_audio_state_get(_player->handle, state);
}
