cmake_minimum_required(VERSION 3.16)
# D15-A #112 opt-in one-screen TEST-ONLY preview; default NORMAL unchanged.
option(WARSHIP_D15A_G01_TEST_ONLY "Isolated G01 visual reception firmware (no NVS/UI)" OFF)
option(WARSHIP_GUI01_TEST_ONLY "Isolated GUI01 interactive G01 button reception, no navigation/NVS" OFF)
if(WARSHIP_GUI01_TEST_ONLY AND WARSHIP_D15A_G01_TEST_ONLY)
  message(FATAL_ERROR "GUI01 tactile and D15-A static test modes are mutually exclusive")
endif()
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(cyd_warship)
