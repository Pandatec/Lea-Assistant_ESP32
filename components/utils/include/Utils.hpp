#ifndef UTILS_HPP
#define UTILS_HPP

#include "more_esp_err.hpp"

#include <map>
#include <string>
#include <vector>

namespace lea::utils {
    void split(std::string str, std::string delimiter, std::vector<std::string> &vec);
    void printStringVector(const std::string &name, std::vector<std::string> &vec);
    void printStringMap(const std::string &name, std::map<std::string, std::string> &dic);
}  // namespace lea::utils

#endif  // UTILS_HPP
