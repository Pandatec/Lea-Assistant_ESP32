#include "SD.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>

extern "C" {
#include "diskio_impl.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "ff.h"
#include "sdkconfig.h"
#include "sdmmc_cmd.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>
}

static const char *TAG = "SD_CARD";

static const std::string MOUNT_POINT = "/sdcard";

static const gpio_num_t PIN_SD_CMD = GPIO_NUM_15;
static const gpio_num_t PIN_SD_D0 = GPIO_NUM_2;
static const gpio_num_t PIN_SD_D1 = GPIO_NUM_4;
static const gpio_num_t PIN_SD_D2 = GPIO_NUM_12;
static const gpio_num_t PIN_SD_D3 = GPIO_NUM_13;

lea::SD::SD()
{
}

esp_err_t lea::SD::init()
{
    esp_err_t ret;
    sdmmc_card_t *card;

    const esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 15,
        .allocation_unit_size = 16 * 1024  //
    };

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;

    ESP_LOGI(TAG, "Initializing SD card");

    gpio_set_pull_mode(PIN_SD_CMD, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_SD_D0, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_SD_D1, GPIO_PULLUP_ONLY);

    ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT.c_str(), &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SD card");
        ESP_LOGE(TAG, "%s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "SD card initialization OK");

    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

std::optional<std::vector<std::string>> lea::SD::listDir(const std::string &dp)
{
    std::vector<std::string> dir_content;

    DIR *dir = opendir(dp.c_str());

    if (dir == NULL) {
        ESP_LOGE(TAG, "Could not open dir %s", dp.c_str());
        return {};
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (std::string(entry->d_name) == "." && std::string(entry->d_name) == "..") {
            continue;
        }
        dir_content.push_back(entry->d_name);
    }
    closedir(dir);

    return dir_content;
}

std::vector<std::string> lea::SD::getFileContent(const std::string &fp)
{
    std::vector<std::string> fc;

    std::string line;
    std::ifstream myfile(fp);

    if (myfile.is_open()) {
        while (getline(myfile, line)) {
            fc.push_back(line);
        }
        myfile.close();
    } else {
        ESP_LOGE(TAG, "Could not open file : %s", fp.c_str());
    }
    return fc;
}

bool lea::SD::fileExists(const std::string &fp)
{
    struct stat sb;

    if (stat(fp.c_str(), &sb) == -1) {
        return false;
    }

    return S_ISREG(sb.st_mode);
}

bool lea::SD::dirExists(const std::string &dp)
{
    struct stat sb;

    if (stat(dp.c_str(), &sb) == -1) {
        return false;
    }

    return S_ISDIR(sb.st_mode);
}

esp_err_t lea::SD::deleteFile(const std::string &fp)
{
    if (remove(fp.c_str())) {
        ESP_LOGE(TAG, "Error removing file: %s", fp.c_str());
        perror(fp.c_str());
        return ESP_FAIL;
    }

    return ESP_OK;
}

void lea::SD::recursiveDeleteDir(const std::string &dp)
{
    struct dirent *entry = NULL;
    DIR *dir = NULL;

    dir = opendir(dp.c_str());

    while ((entry = readdir(dir))) {
        DIR *sub_dir = NULL;
        FILE *file = NULL;

        if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
            std::string abs_path = dp + "/" + std::string(entry->d_name);

            if ((sub_dir = opendir(abs_path.c_str()))) {
                closedir(sub_dir);
                recursiveDeleteDir(abs_path);
            } else {
                if ((file = fopen(abs_path.c_str(), "r"))) {
                    fclose(file);
                    ESP_LOGI(TAG, "removing: %s", abs_path.c_str());
                    remove(abs_path.c_str());
                }
            }
        }
    }
    closedir(dir);

    ESP_LOGI(TAG, "removing: %s", dp.c_str());
    remove(dp.c_str());
}

esp_err_t lea::SD::deleteDir(const std::string &dp)
{
    if (!dirExists(dp)) {
        return ESP_ERR_NOT_FOUND;
    }

    recursiveDeleteDir(dp);

    return ESP_OK;
}

esp_err_t lea::SD::createFile(const std::string &fp)
{
    FILE *file = fopen(fp.c_str(), "w");

    if (!file) {
        ESP_LOGE(TAG, "Error creating file: %s", fp.c_str());
        return ESP_FAIL;
    }

    fclose(file);

    return ESP_OK;
}

esp_err_t lea::SD::createDir(const std::string &dp)
{
    if (mkdir(dp.c_str(), 0777) != 0) {
        ESP_LOGE(TAG, "Could not create directory");
        perror(dp.c_str());
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t lea::SD::writeFile(const std::string &fp, const std::string &fc, bool append)
{
    // TODO

    return ESP_OK;
}

esp_err_t lea::SD::writeFile(const std::string &fp, std::vector<std::string> &fc, bool append)
{
    // TODO

    return ESP_OK;
}

esp_err_t lea::SD::renameFile(const std::string &oldFp, const std::string &newFp)
{
    if (rename(oldFp.c_str(), newFp.c_str()) == 0) {
        ESP_LOGI(TAG, "Renamed file %s to %s", oldFp.c_str(), newFp.c_str());
    } else {
        ESP_LOGE(TAG, "Error renaming file %s to %s", oldFp.c_str(), newFp.c_str());
        perror(NULL);
    }

    return ESP_OK;
}

esp_err_t lea::SD::renameDir(const std::string &oldDp, const std::string &newDp)
{
    if (rename(oldDp.c_str(), newDp.c_str()) == 0) {
        ESP_LOGI(TAG, "Renamed directory %s to %s", oldDp.c_str(), newDp.c_str());
    } else {
        ESP_LOGE(TAG, "Error renaming directory %s to %s", oldDp.c_str(), newDp.c_str());
        perror(NULL);
    }

    return ESP_OK;
}
