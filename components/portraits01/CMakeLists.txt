# #128 isolated TEST-ONLY compositor; no candidate pixels in normal builds.
if(WARSHIP_PORTRAITS01_TEST_ONLY)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  set(P01_SCRIPT "${PROJECT_DIR}/tools/portraits01_generate.py")
  set(P01_ROOT "${PROJECT_DIR}/draft/portraits/pilote_A_L1_C1")
  set(P01_DATA "${CMAKE_CURRENT_BINARY_DIR}/portraits01_data.c")
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
       "${P01_SCRIPT}" "${P01_ROOT}/pilote_32.png"
       "${P01_ROOT}/pilote_48.png" "${P01_ROOT}/pilote_64.png"
       "${P01_ROOT}/comparatif_320x240.png")
  execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${P01_SCRIPT}"
            --repo "${PROJECT_DIR}" --output "${P01_DATA}"
            --report "${CMAKE_CURRENT_BINARY_DIR}/portraits01_sources.json"
    RESULT_VARIABLE P01_EXIT OUTPUT_VARIABLE P01_LOG ERROR_VARIABLE P01_ERR)
  if(NOT P01_EXIT EQUAL 0)
    message(FATAL_ERROR "PORTRAITS01 source freeze failed: ${P01_ERR}")
  endif()
  message(STATUS "${P01_LOG}")
  idf_component_register(SRCS "portraits01_render.c" "${P01_DATA}"
                         INCLUDE_DIRS "include")
  target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
else()
  idf_component_register(INCLUDE_DIRS "include")
endif()
