#include "../include/SD.hpp"
#include "esp_err.h"
#include "esp_log.h"
#include "unity.h"

#include <algorithm>
#include <string>
#include <vector>

static const char *TAG = "Test SD";

lea::SD sd_card;

TEST_CASE("sd init", "sd")
{
    esp_err_t ret;

    ret = sd_card.init();

    TEST_ASSERT(ret == ESP_OK);

    ret = sd_card.deleteDir("/sdcard/titi"); // just in case it was here before
}

TEST_CASE("check test dir exists and make it", "sd")
{
    esp_err_t ret;

    ret = sd_card.createDir("/sdcard/titi");
    TEST_ASSERT(ret == ESP_OK);
}

TEST_CASE("create a file", "sd")
{
    esp_err_t ret;
    bool res;

    ret = sd_card.createFile("/sdcard/titi/file1.txt");
    TEST_ASSERT(ret == ESP_OK);

    res = sd_card.fileExists("/sdcard/titi/file1.txt");
    TEST_ASSERT(res == true);
}

TEST_CASE("rename a file", "sd")
{
    esp_err_t ret;
    bool res;

    ret =
        sd_card.renameFile("/sdcard/titi/file1.txt", "/sdcard/titi/file2.txt");
    TEST_ASSERT(ret == ESP_OK);

    res = sd_card.fileExists("/sdcard/titi/file1.txt");
    TEST_ASSERT(res == false);

    res = sd_card.fileExists("/sdcard/titi/file2.txt");
    TEST_ASSERT(res == true);
}

TEST_CASE("delete a file", "sd")
{
    esp_err_t ret;
    bool res;

    ret = sd_card.deleteFile("/sdcard/titi/file2.txt");
    TEST_ASSERT(ret == ESP_OK);

    res = sd_card.fileExists("/sdcard/titi/file2.txt");
    TEST_ASSERT(res == false);
}

TEST_CASE("list a dir", "sd")
{
    esp_err_t ret;
    bool res;

    ret = sd_card.createFile("/sdcard/titi/file1.txt");
    TEST_ASSERT(ret == ESP_OK);

    ret = sd_card.createFile("/sdcard/titi/file2.txt");
    TEST_ASSERT(ret == ESP_OK);

    ret = sd_card.createFile("/sdcard/titi/file3.txt");
    TEST_ASSERT(ret == ESP_OK);

    auto ct = sd_card.listDir("/sdcard/titi");
    if (ct) {
        std::vector<std::string> vec = *ct;
        std::sort(vec.begin(), vec.end());

        TEST_ASSERT(
            vec.at(0) == std::string("FILE1.TXT")); // upper case bcause FAT32
        TEST_ASSERT(vec.at(1) == std::string("FILE2.TXT"));
        TEST_ASSERT(vec.at(2) == std::string("FILE3.TXT"));

    } else {
        TEST_FAIL();
    }
}

TEST_CASE("delete test dir", "sd") // MUST BE THE LAST TEST
{
    esp_err_t ret;
    bool res;

    res = sd_card.dirExists("/sdcard/titi");
    TEST_ASSERT(res == true);

    ret = sd_card.deleteDir("/sdcard/titi");
    TEST_ASSERT(ret == ESP_OK);

    res = sd_card.dirExists("/sdcard/titi");
    TEST_ASSERT(res == false);
}
