cmake_minimum_required(VERSION 3.16)
# D14 color A/B probe - global configure option so every component sees
# the same test-only decision across ESP-IDF two-pass CMake configure.
option(WARSHIP_D14_COLOR_DIAGNOSTIC
       "D14 isolated LCD color test only; no gameplay or NVS" OFF)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(cyd_warship)
