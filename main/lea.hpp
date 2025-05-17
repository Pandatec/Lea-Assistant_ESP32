#ifndef LEA_HPP
#define LEA_HPP

#include "AudioPlayer.hpp"
#include "Micro.hpp"
#include "SD.hpp"
#include "WebSockets.hpp"
#include "Wifi.hpp"

#include <map>
#include <memory>
#include <string>

namespace lea {

    typedef struct {
        std::unique_ptr<AudioPlayer> player;
        std::unique_ptr<Micro> mic;
        std::unique_ptr<Wifi> wifi;
        std::unique_ptr<WebSocket> webSocket;
        std::map<std::string, std::string> config;
    } data_t;

    std::unique_ptr<data_t> init();
    void run(data_t &data);
}  // namespace lea

#endif  // LEA_HPP
