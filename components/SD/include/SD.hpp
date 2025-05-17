#ifndef SD_HPP
#define SD_HPP

extern "C" {
#include "driver/gpio.h"
#include "esp_err.h"
}

#include <optional>
#include <string>
#include <vector>

namespace lea {
    class SD {
      public:
        SD();

        static esp_err_t init();

        static std::optional<std::vector<std::string>> listDir(const std::string &path);

        static std::vector<std::string> getFileContent(const std::string &fp);

        static bool fileExists(const std::string &fp);

        static bool dirExists(const std::string &dp);

        static esp_err_t deleteFile(const std::string &fp);

        static esp_err_t deleteDir(const std::string &dp);

        static esp_err_t createFile(const std::string &fp);

        static esp_err_t createDir(const std::string &dp);

        static esp_err_t writeFile(const std::string &fp, const std::string &fc, bool append = false);

        static esp_err_t writeFile(const std::string &fp, std::vector<std::string> &fc, bool append = false);

        static esp_err_t renameFile(const std::string &oldFp, const std::string &newFp);

        static esp_err_t renameDir(const std::string &oldDp, const std::string &newDp);

      private:
        static void recursiveDeleteDir(const std::string &dp);
    };
}  // namespace lea

#endif  // SD_HPP
