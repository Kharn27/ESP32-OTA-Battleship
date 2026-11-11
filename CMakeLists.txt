cmake_minimum_required(VERSION 3.16)
# D15-A #112 opt-in one-screen TEST-ONLY preview; default NORMAL unchanged.
option(WARSHIP_D15A_G01_TEST_ONLY "Isolated G01 visual reception firmware (no NVS/UI)" OFF)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(cyd_warship)
