# No RGB assets or Pillow dependency in the default NORMAL image.
# D15-A is opt-in preview only, independent from blocked D14 G03.
if(NOT WARSHIP_D15A_G01_TEST_ONLY)
  idf_component_register(INCLUDE_DIRS "include")
  return()
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(G01_SCRIPT "${PROJECT_DIR}/tools/d15a_generate_g01.py")
set(G01_MANIFEST "${PROJECT_DIR}/assets/manifest.json")
set(G01_ASSET_FILES
    "${PROJECT_DIR}/assets/wallpaper/accueil_retenu_CYD_320x240.png"
    "${PROJECT_DIR}/assets/logo/logo_titre_CYD_145x66_RGBA.png"
    "${PROJECT_DIR}/assets/boutons/bouton_jouer_CYD_140x32_RGBA.png"
    "${PROJECT_DIR}/assets/boutons/bouton_palmares_CYD_140x30_RGBA.png")
set(G01_GENERATED "${CMAKE_CURRENT_BINARY_DIR}/g01_layers.c")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             "${G01_SCRIPT}" "${G01_MANIFEST}" ${G01_ASSET_FILES})
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${G01_SCRIPT}"
          --repo "${PROJECT_DIR}" --output "${G01_GENERATED}"
  RESULT_VARIABLE G01_EXIT OUTPUT_VARIABLE G01_METRICS
  ERROR_VARIABLE G01_ERROR)
if(NOT G01_EXIT EQUAL 0)
  message(FATAL_ERROR "D15-A four approved G01 assets invalid: ${G01_ERROR}")
endif()
message(STATUS "D15-A TEST-ONLY Flash G01: ${G01_METRICS}")
idf_component_register(
  SRCS "g01_compose.c" "${G01_GENERATED}"
  INCLUDE_DIRS "include")
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
