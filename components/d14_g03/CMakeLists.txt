# D14: keep generated, immutable Flash data in the SAME archive as its
# decoder. Avoid unresolved C symbols caused by main/component archive order.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(D14_PNG "${PROJECT_DIR}/design/ecrans/deploiement/g03_deploiement_320x240.png")
set(D14_SVG "${PROJECT_DIR}/design/ecrans/deploiement/g03_deploiement_320x240.svg")
set(D14_GENERATOR "${PROJECT_DIR}/tools/d14_export.py")
set(D14_GENERATED "${CMAKE_CURRENT_BINARY_DIR}/d14_g03_data.c")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             "${D14_PNG}" "${D14_SVG}" "${D14_GENERATOR}")
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${D14_GENERATOR}"
          --png "${D14_PNG}" --svg "${D14_SVG}"
          --output "${D14_GENERATED}"
  RESULT_VARIABLE D14_RESULT OUTPUT_VARIABLE D14_METRICS
  ERROR_VARIABLE D14_ERROR)
if(NOT D14_RESULT EQUAL 0)
  message(FATAL_ERROR "D14 G03 asset extraction failed: ${D14_ERROR}")
endif()
message(STATUS "D14 G03 canonical Flash assets: ${D14_METRICS}")

idf_component_register(SRCS "d14_g03.c" "${D14_GENERATED}"
                       INCLUDE_DIRS "include"
                       REQUIRES warship_deploy_ui t03_decode)
