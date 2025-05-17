#include "Utils.hpp"

static const char *TAG = "UTILS";

void lea::utils::split(std::string str, std::string delimiter, std::vector<std::string> &vec)
{
    size_t pos = 0;
    std::string token;

    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        vec.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    if (!str.empty()) {
        vec.push_back(str);
    }
}

void lea::utils::printStringVector(const std::string &name, std::vector<std::string> &vec)
{
    ESP_LOGI(TAG, "%s", name.c_str());
    for (int i = 0; i < vec.size(); i++) {
        ESP_LOGI(TAG, "%s", vec.at(i).c_str());
    }
    ESP_LOGI(TAG, "============");
}

void lea::utils::printStringMap(const std::string &name, std::map<std::string, std::string> &dic)
{
    ESP_LOGI(TAG, "%s", name.c_str());
    for (auto &[key, value] : dic) {
        ESP_LOGI(TAG, "{key:%s, value:%s}", key.c_str(), value.c_str());
    }
    ESP_LOGI(TAG, "============");
}
