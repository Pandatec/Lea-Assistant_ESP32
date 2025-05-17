cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 17)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS "../components/" $ENV{ADF_PATH}/components)

set(TEST_COMPONENTS
    "SD"
    )

project(lea_firmware_test)
