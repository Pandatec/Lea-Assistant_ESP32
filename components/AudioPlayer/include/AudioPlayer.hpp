#ifndef MP3_PLAYER_HPP
#define MP3_PLAYER_HPP

extern "C" {
#include "esp_audio.h"
#include "esp_err.h"
}
#include <string>

namespace lea {
    class AudioPlayer {
      public:
        /**
         * @brief Create and AudioPlayer
         */
        AudioPlayer();

        /* init all audio stack */
        esp_err_t init();

        /**
         * @brief uninit audio stack
         */
        ~AudioPlayer();

        /**
         * @brief play a music from the sdcard
         *
         * @param fp file path to audio file (without /sdcard). ex:
         * musics/music1.mp3
         *
         * @return ESP_ERR_AUDIO_NO_ERROR or the error if one occured
         */
        audio_err_t music_play(const std::string &fp);

        /**
         * @brief Stop music
         *
         * @return ESP_ERR_AUDIO_NO_ERROR or the error if one occured
         */
        audio_err_t music_stop();

        /**
         * @brief pause music
         *
         * @return OK or Error
         * @return ESP_ERR_AUDIO_NO_ERROR or the error if one occured
         */
        audio_err_t music_pause();

        /**
         * @brief resume music
         *
         * @return ESP_ERR_AUDIO_NO_ERROR or the error if one occured
         */
        audio_err_t music_resume();

        /**
         * @brief get music volume
         *
         * @param vol pointer to int where the voume will be stored
         *
         */
        void vol_get(int *vol);

        /**
         * @brief set music volume
         *
         * @param vol new volume must be from 0 to 100
         */
        void vol_set(int vol);

        /**
         * @brief get player state
         *
         * @param state state to fill
         *
         * @return ESP_ERR_AUDIO_NO_ERROR or the error if one occured
         */
        audio_err_t get_state(esp_audio_state_t *state);

      private:
        /**
         * @brief actual function that play the music
         *
         * @param url url of the music, for sd it's file:/
         * @param pos starting posision in sound
         * @param type source type (SD for us)
         *
         * @return
         */
        audio_err_t music_play(const char *url, int pos, media_source_type_t type);

        /**
         * @brief get player's state and is used by most of the functions
         *
         * @param state player's state
         *
         * @return ESP_ERR_AUDIO_NO_ERROR or the error if one occured
         */
        audio_err_t state_get(esp_audio_state_t *state);

        /**
         * @brief structure for all mp3 player data
         */
        typedef struct {
            esp_audio_handle_t handle;
            esp_audio_info_t backup_info;
            bool backup_flag;
        } esp_player_t;

        esp_player_t *_player;

        static bool isInit;
    };
}  // namespace lea

#endif
